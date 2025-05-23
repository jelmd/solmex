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
#include <kstat.h>
#include <sys/loadavg.h>
#include <rpcsvc/rstat.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/swap.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>

#include <libprom/prom.h>

#include "load.h"
#include "ks_util.h"

// usr/src/cmd/powertop/common/cpufreq.c
// usr/src/cmd/cpc/common/cpustat.c
// usr/src/cmd/cpc/common/cputrack.c
// usr/src/cmd/zonestat/zonestatd/zonestatd.c
// usr/src/lib/libkstat/common/kstat.c
// usr/src/uts/common/io/kstat.c

typedef enum ks_info_idx {
	KS_IDX_LOADAVERAGE = 0,
	KS_IDX_PSET,
	KS_IDX_PROCQ,
	KS_IDX_SWAP,
	KS_IDX_MAX,			// last entry by cotract
} ks_info_idx_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	{ "unix", 0, "system_misc", -1, 0, NULL },
	{ "unix", 0, "pset", -1, 0, NULL },
	{ "unix", 0, "sysinfo", -1, 0, NULL },
	{ "unix", 0, "vminfo", -1, 0, NULL },
};
#pragma GCC diagnostic pop

// see also usr/src/uts/common/os/clock.c
// usr/src/uts/common/sys/cpuvar.h
void
collect_load(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now) {
	static double aven[3] = { 0, 0 , 0 };
	static hrtime_t last_time = 0;
	static uint64_t deficit	= UINT64_MAX;
	char buf[32];

	PROM_DEBUG("collect_load ...", "");

	// make sure we do not stress it too much on direct load - this is the
	// only reason why aven has been made static.
	if ((last_time + NANOSEC) < now) {
		if (kc == NULL) {
			getloadavg(aven, 3);
		} else {
			kstat_t *ksp;
			kstat_named_t *knp;
			ks_info_idx_t idx = KS_IDX_LOADAVERAGE;

			if (update_instance(kc, &kstat[idx]) != 1)
				return;

			if ((ksp = ks_read(kc, kstat[idx].ksp[0], now, NULL)) == NULL)
				return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
			if ((knp = kstat_data_lookup(ksp, "avenrun_1min")) != NULL) {
				aven[LOADAVG_1MIN] = 1.0 * knp->value.i32 / FSCALE;
			}
			if ((knp = kstat_data_lookup(ksp, "avenrun_5min")) != NULL) {
				aven[LOADAVG_5MIN] = 1.0 * knp->value.i32 / FSCALE;
			}
			if ((knp = kstat_data_lookup(ksp, "avenrun_15min")) != NULL) {
				aven[LOADAVG_15MIN] = 1.0 * knp->value.i32 / FSCALE;
			}
			if ((knp = kstat_data_lookup(ksp, "deficit")) != NULL) {
				deficit = knp->value.ui32;
			}
#pragma GCC diagnostic pop
		}
		last_time = now;
	}

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (!compact)
		addPromInfo(SOLMEXM_LOAD1);
	psb_add_str(sb, SOLMEXM_LOAD1_N " ");
	sprintf(buf, "%.17g\n", aven[LOADAVG_1MIN]);
	psb_add_str(sb, buf);

	if (!compact)
		addPromInfo(SOLMEXM_LOAD5);
	psb_add_str(sb, SOLMEXM_LOAD5_N " ");
	sprintf(buf, "%.17g\n", aven[LOADAVG_5MIN]);
	psb_add_str(sb, buf);

	if (!compact)
		addPromInfo(SOLMEXM_LOAD15);
	psb_add_str(sb, SOLMEXM_LOAD15_N " ");
	sprintf(buf, "%.17g\n", aven[LOADAVG_15MIN]);
	psb_add_str(sb, buf);

	if (deficit != UINT64_MAX) {
		if (!compact)
			addPromInfo(SOLMEXM_DEFICIT);
		psb_add_str(sb, SOLMEXM_DEFICIT_N " ");
		sprintf(buf, "%ld\n", deficit << page_shift);
		psb_add_str(sb, buf);
	}
	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_load done", "");
}

#undef swap_alloc
#undef swap_free

// the difference between 2 kstat samples round()ed.
#define INFO_DIFF(x) info[D].x = \
	(0.5 * info[D].updates + (info[next].x - info[prev].x)) / info[D].updates;

typedef enum {
	A = 0,
	B,
	D,
	UNKNOWN = -1
} info_t;

// not thread-safe !
void
collect_procq(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now) {
	char buf[32];
	static sysinfo_t info[3];
	static info_t prev = UNKNOWN;
	info_t next;

	kstat_t *ksp;
	ks_info_idx_t idx = KS_IDX_PROCQ;

	uint8_t n = update_instance(kc, &kstat[idx]);
	if (n != 1)
		return;

	next = (prev + 1) & 0x1;
	if ((ksp = ks_read(kc, kstat[idx].ksp[0], now, &(info[next]))) == NULL)
		return;

	if (prev == UNKNOWN)	{
		// does not make sense w/o a previous stat
		prev = next;
		return;
	}
	// If a system has 16 TiB and sys pages are 4 KiB = 8 TiB = 2^32 pages and
	// thus room for 2^64 updates (it gets made every second) =~ 136 years.
	info[D].updates = 1;
	INFO_DIFF(updates);
	if (info[D].updates < 1)
		info[D].updates = 1;
	INFO_DIFF(runque);
	INFO_DIFF(swpque);
	INFO_DIFF(waiting);
	prev = next;

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (!compact)
		addPromInfo(SOLMEXM_PROCQ_RUN);
	psb_add_str(sb, SOLMEXM_PROCQ_RUN_N " ");
	sprintf(buf, "%u\n", info[D].runque);
	psb_add_str(sb, buf);

	if (!compact)
		addPromInfo(SOLMEXM_PROCQ_SWAP);
	psb_add_str(sb, SOLMEXM_PROCQ_SWAP_N " ");
	sprintf(buf, "%u\n", info[D].swpque);
	psb_add_str(sb, buf);

	if (!compact)
		addPromInfo(SOLMEXM_PROCQ_WAIT);
	psb_add_str(sb, SOLMEXM_PROCQ_WAIT_N " ");
	sprintf(buf, "%u\n", info[D].waiting);
	psb_add_str(sb, buf);

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_procq done", "");
}

#define PSHIFT page_shift

// not thread-safe !
void
collect_swap(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now) {
	char buf[32];
	static vminfo_t info[3];
	static info_t prev = UNKNOWN;
	info_t next;
	//uint64_t used;

	kstat_t *ksp;
	ks_info_idx_t idx = KS_IDX_SWAP;

	if (kc == NULL) {
		struct anoninfo ai;
		if (swapctl(SC_AINFO, &ai) == -1) {
			PROM_WARN("SWAP info n/a: ", strerror(errno));
			return;
		}
		memset(&(info[D]), 0, sizeof(vminfo_t));
		// ani.max = total amount of swap space including free physical memory
		// ani.free = amount of unallocated anonymous memory
		// ani_resv = total amount of reserved anonymous memory
		info[D].swap_resv = ai.ani_resv;
		info[D].swap_free = ai.ani_free;
		info[D].swap_avail = ai.ani_max - ai.ani_resv;
		info[D].swap_alloc = ai.ani_max - ai.ani_free;
		// allocated = swap_avail + swap_resv - swap_free	= ai.ani_max - ai.ani_free
		// reserved = swap_free - swap_avail				= ai.ani_resv - allocated
		// used = swap_resv									= allocated + reserved
		// available = swap_avail							= ai.ani_max - ai.ani_resv
	} else {
		int n = update_instance(kc, &kstat[idx]);
		if (n != 1)
			return;

		next = (prev + 1) & 0x1;
		if ((ksp = ks_read(kc, kstat[idx].ksp[0], now, &(info[next]))) == NULL)
			return;

		if (prev == UNKNOWN)	{
			// does not make sense w/o a previous stat
			prev = next;
			return;
		}
		// If a system has 16 TiB and sys pages are 4 KiB = 8 TiB = 2^32 pages and
		// thus room for 2^64 updates (it gets made every second) =~ 136 years.
		info[D].updates = 1;
		INFO_DIFF(updates);
		if (info[D].updates < 1)
			info[D].updates = 1;
		// avail_rmem = MAX(availrmem - swapfs_minfree, 0)
		INFO_DIFF(swap_resv);	// k_anoninfo.(ani_phys_resv + ani_mem_resv);
		INFO_DIFF(swap_alloc);	// k_anoninfo.(ani_mem_resv + ani_max - ani_free);
		INFO_DIFF(swap_avail);	// avail_rmem + k_anoninfo.(ani_max - ani_phys_resv);
		INFO_DIFF(swap_free);	// avail_rmem + k_anoninfo.ani_free;
		prev = next;
	}

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (!compact)
		addPromInfo(SOLMEXM_SWAP_RESV);
	psb_add_str(sb, SOLMEXM_SWAP_RESV_N " ");
	sprintf(buf, "%ld\n", info[D].swap_resv << PSHIFT);
	psb_add_str(sb, buf);

	if (!compact)
		addPromInfo(SOLMEXM_SWAP_ALLOC);
	psb_add_str(sb, SOLMEXM_SWAP_ALLOC_N " ");
	sprintf(buf, "%ld\n", info[D].swap_alloc << PSHIFT);
	psb_add_str(sb, buf);

	if (!compact)
		addPromInfo(SOLMEXM_SWAP_AVAIL);
	psb_add_str(sb, SOLMEXM_SWAP_AVAIL_N " ");
	sprintf(buf, "%ld\n", info[D].swap_avail << PSHIFT);
	psb_add_str(sb, buf);

	if (!compact)
		addPromInfo(SOLMEXM_SWAP_FREE);
	psb_add_str(sb, SOLMEXM_SWAP_FREE_N" ");
	sprintf(buf, "%ld\n", info[D].swap_free << PSHIFT);
	psb_add_str(sb, buf);

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_swap done", "");
}

/*
node_cpus_total{state="offline"} 0
node_cpus_total{state="online"} 24
*/
void
collect_cpu_state(psb_t *sb, bool compact, hrtime_t now) {
	static hrtime_t last_time = 0;
	static uint64_t all, online;
	char buf[32];

	PROM_DEBUG("collect_cpu_state ...", "");

	// NOTE that the unix::system_misc:nproc shows all populated sockets whereby
	// unix::pset:nproc show the enabled CPUs per processor group (which is often
	// just one), only (what we want). However, they are not zone aware and
	// therefore we use sysconf to obtain the values.
	if ((last_time + NANOSEC) < now) {
		online = sysconf(_SC_NPROCESSORS_ONLN);
		all = sysconf(_SC_NPROCESSORS_CONF);
	}

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (!compact)
		addPromInfo(SOLMEXM_CPUSTATE);

	psb_add_str(sb, SOLMEXM_CPUSTATE_N "{state=\"online\"} ");
	sprintf(buf, "%ld", online);
	psb_add_str(sb, buf);
	psb_add_char(sb, '\n');
	psb_add_str(sb, SOLMEXM_CPUSTATE_N "{state=\"offline\"} ");
	sprintf(buf, "%ld", all - online);
	psb_add_str(sb, buf);
	psb_add_char(sb, '\n');

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_cpu_state done", "");
}

void
collect_units(psb_t *sb, bool compact) {
	static char *metrics = NULL;

	char buf[32];

	PROM_DEBUG("collect_cpu_state ...", "");

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (metrics == NULL) {
		size_t pos = psb_len(sb);

		if (!compact)
			addPromInfo(SOLMEXM_UNIT_PAGE);
		psb_add_str(sb, SOLMEXM_UNIT_PAGE_N " ");
		sprintf(buf, "%ld\n", 1L << page_shift);
		psb_add_str(sb, buf);
		if (!compact)
			addPromInfo(SOLMEXM_UNIT_TICKS);
		psb_add_str(sb, SOLMEXM_UNIT_TICKS_N " ");
		sprintf(buf, "%ld\n", tps);
		psb_add_str(sb, buf);
		metrics = strdup(psb_str(sb) + pos);
	} else {
		psb_add_str(sb, metrics);
	}

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_units done", "");
}
