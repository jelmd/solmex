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

#include "boottime.h"

static char *boot_time;		// node_boot_time_seconds 1.741802979e+09

static void
buildMetric(psb_t *sb, bool compact) {
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kid_t kid;
	kstat_named_t *knp;

	size_t sz;
	char *buf;

	sz = psb_len(sb);
	if (!compact)
		addPromInfo(SOLMEXM_BOOTTIME);
	while ((kc = kstat_open()) == NULL) {
		if (errno == EAGAIN) {
			(void) poll(NULL, 0, 200);
		} else {
			buf = strerror(errno);
			PROM_WARN("Unable to access kstats: %s", buf)
			break;
		}
	}
	if (kc == NULL) {
		psb_truncate(sb, sz);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
	} else if ((ksp = kstat_lookup(kc, "unix", -1, "system_misc")) == NULL) {
#pragma GCC diagnostic pop
		PROM_WARN("kstat unix:0:system_misc n/a", "")
		psb_truncate(sb, sz);
	} else {
		while ((kid = kstat_read(kc, ksp, NULL)) == -1) {
			if (errno == EAGAIN) {
				(void) poll(NULL, 0, 200);
			} else {
				buf = strerror(errno);
				PROM_WARN("kstat unix:0:system_misc read error: %s", buf)
				break;
			}
		}
		if (kid == -1) {
			psb_truncate(sb, sz);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
		} else if ((knp = kstat_data_lookup(ksp, "boot_time")) != NULL) {
#pragma GCC diagnostic pop
			char val[32];	// 16 is sufficient for the next ~80 years
			uint64_t v = 0;
			if (knp->data_type == KSTAT_DATA_UINT32) {
				v = knp->value.ui32;
			} else if (knp->data_type == KSTAT_DATA_UINT64) {
				v = knp->value.ui64;
			} else if (knp->data_type == KSTAT_DATA_INT32) {
				v = knp->value.i32;
			} else if (knp->data_type == KSTAT_DATA_INT64) {
				v = knp->value.i64;
			} else {
				PROM_WARN("Unexpected boot_time data_type %d", knp->data_type);
			}
			snprintf(val, sizeof(val), "%ld", v);
			psb_add_str(sb, SOLMEXM_BOOTTIME_N " ");
			psb_add_str(sb, val);
			psb_add_char(sb, '\n');
		} else if (!compact) {
			PROM_WARN("kstat unix:0:system_misc:boot_time n/a", "");
			psb_truncate(sb, sz);
		}
	}
	if (kc != NULL)
		kstat_close(kc);

	boot_time = strdup(psb_str(sb) + sz);
}

void
collect_boottime(psb_t *sb, bool compact) {
	PROM_DEBUG("collect_boottime ...", "");

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (boot_time == NULL) {
		buildMetric(sb, compact);
	} else if (boot_time[0] != '\0') {
		psb_add_str(sb, boot_time);
	}

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_boottime done", "");
}
