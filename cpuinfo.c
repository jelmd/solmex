/*
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License") 1.1!
 * You may not use this file except in compliance with the License.
 *
 * See  https://spdx.org/licenses/CDDL-1.1.html  for the specific
 * language governing permissions and limitations under the License.
 *
 * Copyright 2025 Jens Elkner (jel+solmex-src@cs.ovgu.de)
 */

#include <stdio.h>
#include <smbios.h>
#include <kstat.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

#include <libprom/prom.h>

#include "cpuinfo.h"
#include "ks_util.h"
#include "dmi.h"

// see also:
// illumos-gate/usr/src/cmd/psrinfo/psrinfo.c

// chip_id      .. the ID of the CPU alias package alias socket. Enumerated by
//				   the OS starting with 0
// pkg_core_id	.. the ID of the core wrt. to the owning CPU
// core_id      .. the ID of the strand wrt. to the owning CPU
// pg_id	    .. the ID of the core wrt. to the system
// ncore_per_chip .. cores/cpu
// ncpu_per_chip  .. strands/cpu

static char *cpu_info[MAX_CPUS];
static uint64_t seen;
#define HAVE_CPU(id)	(seen & (1 << (id)))
#define ADD_CPU(id)		(seen |= (1 << (id)))
static uint8_t cpu_count = 0;

static bool initialized = false;

static char *
trimCpuModel(const char *brand) {
	char *s, *t;
	size_t len;
	int c;

	if (brand == NULL || (len = strlen(brand)) == 0)
		return NULL;

	s = strdup(brand);
	t = s + len - 2;
	c = 2;
	if (t[0] == 'H' && t[1] == 'z') {
		// strip off the ' @  3.30GHz' - freq* attr contains this info already
		while (t >= s) {
			t--;
			c++;
			if (t[0] == ' ') {
				c = -(c + 1);
				t--;
				// cut off the trailing '@'
				while (t >= s && t[0] != ' ') {
					t--;
					c--;
				}
				// remove trailing whitespaces
				while (t >= s && t[0] == ' ') {
					t--;
					c--;
				}
				t++;
				c++;
				if (t - 2 > s && t[-1] == '0' && t[-2] == ' ')
					c -= 2;		// trailing ' 0' on Intel CPUs
				break;
			}
		}
		if (c < 0) {
			len += c;
			s[len] = '\0';
		}
	}
	t = s + len - 3;
	// s|\(R|r|TM|tm\)| (CPU|Processor)||g
	while (t > s) {
		t--;
		if (t[0] == '(') {
			if ((t[1] == 'r' || t[1] == 'R') && t[2] == ')') {
				memmove(t, t+3, len - (t + 3 - s) + 1);
				t -= 3;
				len -= 3;
			} else if ((t[1] == 't' || t[1] == 'T')
				&& (t[2] == 'm' || t[2] == 'M')  && t[3] == ')')
			{
				memmove(t, t+4, len - (t + 4 - s) + 1);
				t -= 3;
				len -= 4;
			}
			continue;
		}
		if (t[0] == 'C' && t[1] == 'P' && t[2] == 'U' && t > s && t[-1] == ' ') {
			memmove(t-1, t+3, len - (t + 3 - s) + 1);
			t -= 3;
			len -= 4;
		}
		if (t[0] == 'P' && (strncmp(t+1, "rocessor", 8) == 0) && t > s && t[-1] == ' ') {
			memmove(t-1, t+9, len - (t + 9 - s) + 1);
			len -= 9;
		}
	}
	// s| [^ ]*-Core$||
	if (len > 5 && strcmp(s + len - 5, "-Core") == 0) {
		t = s + len - 6;
		while (t >= s && t[0] != ' ')
			t--;
		if (t > s) {
			t[0] = '\0';
			len = t - s;
		}
	}
	// finaly s|  +| |g
	t = s;
	while (t < (s + len)) {
		char *p;
		if (t[0] == ' ') {
			p = t;
			t++;
			while (t[0] == ' ')
				t++;
			c = t - p;
			if (c > 1) {
				memmove(p + 1, t, (s + len) - t + 1);
				len -= c;
			}
		}
		t++;
	}
	return s;
}

static bool
addSpeedInfo(psb_t *sb, const char *frequencies, uint8_t cpuNum) {
	int64_t min = -1, max = -1, turbo_speed = get_turbo_speed(cpuNum);
	char buf[32], *s, *t;
	size_t len;

	if (frequencies != NULL) {
		len = strlen(frequencies);

		if (len == 0)
			return false;

		errno = 0;
		min = strtol(frequencies, NULL, 10);
		if (errno) {
			PROM_WARN("Unable to parse kstat's CPU frequencies from '%s'", frequencies);
			min = -1;
		}
		if ((t = strrchr(frequencies, ':')) != NULL) {
			max = strtol(t, NULL, 10);
			if (errno) {
				PROM_WARN("Unable to extract kstat's CPU max frequency from '%s'", s);
				max = min;
			}
		}
	}
	if (min < 0)
		min = max < 0 ? turbo_speed : max;
	if (max < min)
		max = turbo_speed > min ? turbo_speed : min;

	sprintf(buf, "%ld", min) < 0 ? -1 : min;
	psb_add_str(sb, "freq_min=\"");
	psb_add_str(sb, buf);
	psb_add_str(sb, "\",");
	sprintf(buf, "%ld", max < 0 ? -1 : max);
	psb_add_str(sb, "freq_max=\"");
	psb_add_str(sb, buf);
	psb_add_str(sb, "\",");
	return true;
}

static void
addCpuInfo(psb_t *sb, size_t sz, kstat_t *ksp, uint chip_id) {
	kstat_named_t *knp;
	char buf[32], *s, *t;
	int n;
	size_t len;
	int64_t cache_sz;
	bool freq_done = false;

	psb_truncate(sb, sz);
	psb_add_str(sb, SOLMEXM_CPUINFO_N "{package=\"");
	sprintf(buf, "%d", chip_id);
	psb_add_str(sb, buf);
	psb_add_str(sb, "\",");
	cache_sz = get_cache_size(cpu_count);
	if (cache_sz > 0) {
		psb_add_str(sb, "cachesize=\"");
		sprintf(buf, "%ld", cache_sz >> 10);
		psb_add_str(sb, buf);
		psb_add_str(sb, " KB\",");
	}
	for (n = ksp->ks_ndata, knp = KSTAT_NAMED_PTR(ksp); n > 0; n--, knp++) {
		// NOTE: There is no safe way anymore to extract the CPU 'microcode'
		// version on Solaris. Also usually not needed, so don't care. ;-)
		if (strcmp(knp->name, "brand") == 0) {
			s = trimCpuModel(KSTAT_NAMED_STR_PTR(knp));
			if (s != NULL) {
				psb_add_str(sb, "model_name=\"");
				psb_add_str(sb, s);
				psb_add_str(sb, "\",");
				free(s);
			}
		} else if (strcmp(knp->name, "family") == 0) {
			psb_add_str(sb, "family=\"");
			sprintf(buf, "%d", knp->value.i32);
			psb_add_str(sb, buf);
			psb_add_str(sb, "\",");
		} else if (strcmp(knp->name, "implementation") == 0) {
			s = KSTAT_NAMED_STR_PTR(knp);
			len = strlen(s);
			t = strrchr(s, ' ');
			if ((t < s + len - 4) && t[1] == 'M' && t[2] ==  'H' && t[3] == 'z') {
				t[0] = '\0';
				t--;
				while (t > s && '0' <= t[0] && t[0] <= '9')
					t--;
				if (t[0] == ' ') {
					psb_add_str(sb, "freq_base=\"");
					psb_add_str(sb, t+1);
					psb_add_str(sb, "000000\",");
				}
			}
		} else if (strcmp(knp->name, "ncore_per_chip") == 0) {
			psb_add_str(sb, "cores=\"");
			sprintf(buf, "%d", knp->value.i32);
			psb_add_str(sb, buf);
			psb_add_str(sb, "\",");
		} else if (strcmp(knp->name, "model") == 0) {
			psb_add_str(sb, "model=\"");
			sprintf(buf, "%d", knp->value.i32);
			psb_add_str(sb, buf);
			psb_add_str(sb, "\",");
		} else if (strcmp(knp->name, "stepping") == 0) {
			psb_add_str(sb, "stepping=\"");
			sprintf(buf, "%d", knp->value.i32);
			psb_add_str(sb, buf);
			psb_add_str(sb, "\",");
		} else if (strcmp(knp->name, "supported_frequencies_Hz") == 0) {
			freq_done = addSpeedInfo(sb, KSTAT_NAMED_STR_PTR(knp), cpu_count);
		} else if (strcmp(knp->name, "vendor_id") == 0) {
			psb_add_str(sb, "vendor=\"");
			psb_add_str(sb, KSTAT_NAMED_STR_PTR(knp));
			psb_add_str(sb, "\",");
		}
	}
	if (!freq_done)
		addSpeedInfo(sb, NULL, cpu_count);
	n = psb_len(sb);
	psb_truncate(sb, n - 1);
	psb_add_str(sb, "} 1");
	psb_add_char(sb,'\n');
	cpu_info[cpu_count] = strdup(psb_str(sb) + sz);
	cpu_count++;
	ADD_CPU(chip_id);
}

static void
buildInfoMetric(psb_t *sb) {
	kstat_ctl_t *kc = NULL;
	kstat_t *ksp;
	kstat_named_t *knp;

	size_t sz;
	char *buf;
	uint_t chip_id;
	initialized = true;

	sz = psb_len(sb);
	if (kc == NULL) {
		while ((kc = kstat_open()) == NULL) {
			if (errno == EAGAIN) {
				(void) poll(NULL, 0, 50);
			} else {
				buf = strerror(errno);
				PROM_WARN("Unable to access kstats: %s", buf)
				break;
			}
		}
	}
	if (kc == NULL) {
		psb_truncate(sb, sz);
		return;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
	} else if ((ksp = kstat_lookup(kc, "cpu_info", -1, NULL)) == NULL) {
#pragma GCC diagnostic pop
		PROM_WARN("kstat cpu_info n/a", "")
	} else {
		uint8_t n = 0;
		// we got the pointer to the first 'cpu_info' match. Now we need to
		// go through all remaining records in the chain to find additional
		// records
		for (; ksp; ksp = ksp->ks_next) {
			if (strcmp(ksp->ks_module, "cpu_info") != 0)
				continue;
			if (ks_read(kc, ksp, 0) == NULL)
				continue;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
			if ((knp = kstat_data_lookup(ksp, "chip_id")) == NULL)
				continue;	// should not happen
#pragma GCC diagnostic pop
			n++;
			if (knp->value.i64 >= MAX_CPUS) {
				PROM_WARN("kstat cpu_info:%d:%s:chip_id is ignored (%ld > %d)",
					ksp->ks_instance, ksp->ks_name, knp->value.i64, MAX_CPUS - 1);
				continue;
			}
			chip_id = knp->value.i64;
			if (HAVE_CPU(chip_id))
				continue;
			addCpuInfo(sb, sz, ksp, chip_id);
		}
		system_cpu_count = n;
	}
	psb_truncate(sb, sz);
}

void
collect_cpuinfo(psb_t *sb, bool compact) {
	int n;

	PROM_DEBUG("collect_cpuinfo ...", "");

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (!initialized)
		buildInfoMetric(sb);

	if (!compact)
		addPromInfo(SOLMEXM_CPUINFO);
	for (n = 0; n < cpu_count; n++) {
		if (cpu_info[n] != NULL)
			psb_add_str(sb, cpu_info[n]);
	}

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_cpuinfo done", "");
}
