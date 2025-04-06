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
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

#include "ks_util.h"

/*
node_cpus_total{state="offline"} 0
node_cpus_total{state="online"} 24
*/

// usr/src/cmd/powertop/common/cpufreq.c
// usr/src/cmd/cpc/common/cpustat.c
// usr/src/cmd/cpc/common/cputrack.c
// usr/src/cmd/zonestat/zonestatd/zonestatd.c
// usr/src/lib/libkstat/common/kstat.c
// usr/src/uts/common/io/kstat.c

static const uint32_t MAX_KC_TRIES = 1000 * KS_TIMEOUT / KS_WAIT ;

kstat_ctl_t *
ks_chain_open_or_update(kstat_ctl_t *kc) {
	int32_t count = MAX_KC_TRIES;

	if (kc == NULL) {
		while ((kc = kstat_open()) == NULL) {
			if (errno == EAGAIN) {
				(void) poll(NULL, 0, KS_WAIT);
				errno = 0;
				if (count == 0) {
					count--;
					break;
				}
			} else {
				count = -2;
				break;
			}
		}
	} else {
		while (kstat_chain_update(kc) == -1) {
			if (errno == EAGAIN) {
				(void) poll(NULL, 0, KS_WAIT);
				errno = 0;
				if (count == 0) {
					count = -3;
					break;
				}
			} else {
				count = -4;
				break;
			}
		}
	}
	if (count < 0) {
		char *buf = strerror(errno);
		PROM_WARN("Unable to %s kstats: %s", (count > -3 ? "access" : "update"), buf);
		kstat_close(kc);
		kc = NULL;
	}
	return kc;
}

#define LOAD_MAX_TMP_ENTRIES 128

int
update_instance(kstat_ctl_t *kc, ks_info_t *ks) {
	kstat_t *ksp;

	kstat_t *tmp_ksp[LOAD_MAX_TMP_ENTRIES];
	uint8_t found = 0, i;//, k;

	// entries uptodate ?
	if ((kc->kc_chain_id == ks->last_kid))
		return ks->entries;

	assert(ks->module != NULL || ks->name != NULL);
	if ((ksp = kstat_lookup(kc, ks->module, ks->instance, ks->name)) == NULL) {
		if (errno != ENOENT) {
			PROM_WARN("'%s:%d:%s' is neither a named nor a timer kstat",
				ks->module, ks->instance, (ks->name ? ks->name : "NULL"));
		}
		KS_RESET_INFO(ks);
		ks->last_kid = ks->last_kid;
		ks->entries = 0;
		return 0;
	}
	tmp_ksp[found++] = ksp;
	if (ks->instance < 0 || ks->name == NULL) {
		// wildcards - so try to find more
		for (ksp = ksp->ks_next; ksp; ksp = ksp->ks_next) {
			if (ks->module != NULL && (strcmp(ks->module, ksp->ks_module) != 0))
				continue;
			if (ks->instance >= 0 && ks->instance != ksp->ks_instance)
				continue;
			if (ks->name != NULL && (strcmp(ks->name, ksp->ks_name) != 0))
				continue;
			tmp_ksp[found++] = ksp;
			if (found == LOAD_MAX_TMP_ENTRIES) {
				PROM_WARN("Max instance found (%d) - skipping others", LOAD_MAX_TMP_ENTRIES);
				break;
			}
		}
	}
	// that's why we make all this: we wanna keep already populated instances
	if (ks->entries < found) {
		kstat_t **ksp_new = realloc(ks->ksp, found * sizeof(kstat_t *));
		if (ksp_new == NULL) {
			char *s = strerror(errno);
			PROM_WARN("Unable to alloc ksp array: %s", s);
			KS_RESET_INFO(ks);
			return -1;	// try again later
		}
		ks->ksp = ksp_new;
	}
	/*
	for (i = 0; i < found; i++) {
		kid_t kid_new =  tmp_ksp[i]->ks_kid;
		for (k = 0; k < ks->entries; k++) {
			if (ks->ksp[k]->ks_kid == kid_new)
				tmp_ksp[i] = ks->ksp[k];
		}
	}
	*/
	for (i = 0; i < found; i++)
		ks->ksp[i] = tmp_ksp[i];
	for (i = found; i < ks->entries; i++)
		ks->ksp[i] = NULL;
	ks->entries = found;
	ks->last_kid = kc->kc_chain_id;
	return found;
}

#define MAX_READ_ERRORS 5		// don't wanna block forever

kstat_t *
ks_read(kstat_ctl_t *kc, kstat_t *ksp, hrtime_t now, void *data) {
	kid_t kid;
	int count = 0;

	hrtime_t delta = now - ksp->ks_snaptime;
	if (ksp->ks_data && delta > 0 && delta < NANOSEC)
		return ksp;

	while ((count < MAX_READ_ERRORS) && (kid = kstat_read(kc, ksp, data)) == -1) {
		if (errno == EAGAIN) {
			if (count > 0)
				(void) poll(NULL, 0, KS_READ_WAIT);
			count++;
		} else {
			char *buf = strerror(errno);
			PROM_WARN("kstat 0x%p  %s:%d:%s read error: %s", ksp,
				ksp->ks_module, ksp->ks_instance, ksp->ks_name, buf)
			break;
		}
	}
	return kid == -1 ? NULL : ksp;
}
