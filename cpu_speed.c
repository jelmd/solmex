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

#include <libprom/prom.h>

#include "cpu_speed.h"
#include "ks_util.h"

// usr/src/cmd/powertop/common/cpufreq.c
// usr/src/cmd/cpc/common/cpustat.c
// usr/src/cmd/cpc/common/cputrack.c
// usr/src/cmd/zonestat/zonestatd/zonestatd.c
// usr/src/lib/libkstat/common/kstat.c
// usr/src/uts/common/io/kstat.c

typedef enum ks_info_idx {
	KS_SPEED = 0,
	KS_IDX_MAX,			// last entry by cotract
} ks_info_idx_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	{ "cpu_info", -1, NULL, -1, 0, NULL },
};
#pragma GCC diagnostic pop

void
collect_cpu_speed(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now,
	bool include_max)
{
	kstat_t *ksp;
	kstat_named_t *knp;
	char buf[32];
	size_t sb_pos, len;
	uint64_t freq, freqmax;
	static psb_t *sbmax = NULL;
	bool found;

	PROM_DEBUG("collect_cpu_speed ...", "");

	uint8_t n = update_instance(kc, &kstat[KS_SPEED]);
	if (n < 1)
		return;

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (include_max) {
		if (sbmax == NULL) {
			sbmax = psb_new();
		} else {
			psb_truncate(sbmax, 0);
		}
	}

	if (!compact) {
		if (include_max) {
			psb_t *sbtmp = sb;
			sb = sbmax;
			addPromInfo(SOLMEXM_CPUSPEEDMAX);
			sb = sbtmp;
		}
		addPromInfo(SOLMEXM_CPUSPEED);
	}
	for (int i = 0; i < n; i++) {
		freq = freqmax = -1;
		psb_add_str(sb, SOLMEXM_CPUSPEED_N "{");
		sb_pos = psb_len(sb);
		sprintf(buf, "cpu=\"%d\",", i);
		psb_add_str(sb, buf);
		found = false;
		if ((ksp = ks_read(kc, kstat[KS_SPEED].ksp[i], now)) != NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
			if ((knp = kstat_data_lookup(ksp, "chip_id")) != NULL) {
				sprintf(buf, "package=\"%ld\",", knp->value.i64);
				psb_add_str(sb, buf);
				found = true;
			}
			if ((knp = kstat_data_lookup(ksp, "core_id")) != NULL) {
				sprintf(buf, "core=\"%ld\",", knp->value.i64);
				psb_add_str(sb, buf);
				found = true;
			}
			if ((knp = kstat_data_lookup(ksp, "clog_id")) != NULL) {
				sprintf(buf, "lid=\"%d\",", knp->value.i32);
				psb_add_str(sb, buf);
				found = true;
			}
			if ((knp = kstat_data_lookup(ksp, "current_clock_Hz")) != NULL) {
				freq = knp->value.ui64;
			}
			if (include_max && (knp = kstat_data_lookup(ksp, "supported_frequencies_Hz")) != NULL) {
				char *s = KSTAT_NAMED_STR_PTR(knp);
				char *t;
				freqmax = (t = strrchr(s, ':')) == NULL ? atol(s) : atol(t+1);
				found = true;
			}
#pragma GCC diagnostic pop
		}
		len = psb_len(sb);
		if (found)
			psb_truncate(sb, len -1);
		if (include_max) {
			psb_add_str(sbmax, SOLMEXM_CPUSPEEDMAX_N "{");
			psb_add_str(sbmax, psb_str(sb) + sb_pos);
			sprintf(buf, "} %ld\n", freqmax);
			psb_add_str(sbmax, buf);
		}
		sprintf(buf, "} %ld\n", freq);
		psb_add_str(sb, buf);
	}

	if (include_max)
		psb_add_str(sb, psb_str(sbmax));

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_cpuinfo done", "");
}
