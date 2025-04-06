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

#include "ks_util.h"
#include "mem.h"

typedef enum ks_info_idx {
	KS_IDX_SYSPAGES = 0,
	KS_IDX_MAX,			// last entry by cotract
} ks_info_idx_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	{ "unix", 0, "system_pages", -1, 0, NULL },
};
#pragma GCC diagnostic pop

void
collect_sys_mem(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now) {
	kstat_t *ksp;
	kstat_named_t *knp;
	char buf[32];

	bool free_sb = sb == NULL;
	size_t physmem = 0, lockedmem = 0;

	PROM_DEBUG("collect_sys_mem ...", "");

	uint8_t n = update_instance(kc, &kstat[KS_IDX_SYSPAGES]);
	if (n != 1)
		return;

	if ((ksp = ks_read(kc, kstat[KS_IDX_SYSPAGES].ksp[0], now, NULL)) == NULL)
		return;

	if (free_sb)
		sb = psb_new();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
	if ((knp = kstat_data_lookup(ksp, "physmem")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_PHYSMEM);
		psb_add_str(sb, SOLMEXM_PHYSMEM_N " ");		// same as pagestotal
		physmem = knp->value.ui64 << page_shift;
		sprintf(buf, "%ld\n", physmem);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "availrmem")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_AVAILRMEM);
		psb_add_str(sb, SOLMEXM_AVAILRMEM_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "pageslocked")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_LOCKEDMEM);
		psb_add_str(sb, SOLMEXM_LOCKEDMEM_N " ");
		lockedmem = knp->value.ui64 << page_shift;
		sprintf(buf, "%ld\n", lockedmem);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "freemem")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_FREEMEM);
		psb_add_str(sb, SOLMEXM_FREEMEM_N " ");		// same as pagesfree
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "lotsfree")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_LOTSFREE);
		psb_add_str(sb, SOLMEXM_LOTSFREE_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "desfree")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_DESFREE);
		psb_add_str(sb, SOLMEXM_DESFREE_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "minfree")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_MINFREE);
		psb_add_str(sb, SOLMEXM_MINFREE_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "desscan")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_DESSCAN);
		psb_add_str(sb, SOLMEXM_DESSCAN_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "slowscan")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_SLOWSCAN);
		psb_add_str(sb, SOLMEXM_SLOWSCAN_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "fastscan")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_FASTSCAN);
		psb_add_str(sb, SOLMEXM_FASTSCAN_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "nscan")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_NSCAN);
		psb_add_str(sb, SOLMEXM_NSCAN_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "pp_kernel")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_PPKERNEL);
		psb_add_str(sb, SOLMEXM_PPKERNEL_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64 << page_shift);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "nalloc_calls")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_NALLOC);
		psb_add_str(sb, SOLMEXM_NALLOC_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "nalloc")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_NALLOCSZ);
		psb_add_str(sb, SOLMEXM_NALLOCSZ_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "nfree_calls")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_NFREE);
		psb_add_str(sb, SOLMEXM_NFREE_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64);
		psb_add_str(sb, buf);
	}
	if ((knp = kstat_data_lookup(ksp, "nfree")) != NULL) {
		if (!compact)
			addPromInfo(SOLMEXM_NFREESZ);
		psb_add_str(sb, SOLMEXM_NFREESZ_N " ");
		sprintf(buf, "%ld\n", knp->value.ui64);
		psb_add_str(sb, buf);
	}
#pragma GCC diagnostic pop

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_sys_mem done", "");
}
