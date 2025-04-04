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

/**
 * @file ks_util.h
 * Helper/utilities to make use of non-static kernel stats easier and more
 * readable
 */

#ifndef SOLMEX_KSUTIL_H
#define SOLMEX_KSUTIL_H

#include <kstat.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** time in ms to throttle kc updates/open on EAGAIN */
#define KS_WAIT 100
/** max. time in s to wait until a kstat chain update/open succeeds */
#define KS_TIMEOUT 2

/**
 * @brief Try to open or update a kstat chain taking It is actually a wrapper
 * 	around kstat_open() and kstat_update(), which automagically retries the
 *  desired operation on `EAGAIN` errors for up to `KS_TIMEOUT` seconds.
 * @param kc	Pointer to the kstat chain to update. If `NULL` kstat_open()
 * 	gets used instead off kstat_update chain.
 * @return `NULL` on error, the pointer to the kstat chain otherwise.
 */
kstat_ctl_t *ks_chain_open_or_update(kstat_ctl_t *kc);

/**
 * At least modul or name are required to be != NULL.
 */
typedef struct ks_info {
	char *module;		/**< the module name to lookup. */
	int instance;		/**< the instance to lookup, or -1 if all */
	char *name;			/**< the statistic name to lookup, or NULL if all */
	kid_t last_kid;		/**< Id of the kstat chain where ksp entries belong to */
	uint8_t entries;	/**< number of instances found and stored in ksp below */
	kstat_t **ksp;		/**< the kstat instance[s] holding the related data */
} ks_info_t;

/**
 * @brief Just set entries = 0 and free ksp member, if != NULL
 */
#define KS_RESET_INFO(x) \
	(x)->entries = 0; \
	if ((x)->ksp != NULL) { \
		free((x)->ksp); \
		(x)->ksp = NULL; \
	}

#define KS_PRINT_INFO(x, now) \
	fprintf(stderr, "KSP %s:%d:%s  id: %d  data: %p (%lld - %lld = %lld)\n", \
		(x)->ks_module, (x)->ks_instance, (x)->ks_name, \
		(x)->ks_kid, (x)->ks_data, \
		now, (x)->ks_snaptime, (now - (x)->ksp->ks_snaptime));

#define KS_PRINT_INSTANCE(x) \
	fprintf(stderr, "KSP %d: %s:%d:%s has %ld bytes in %d entries (type: %d)\n", \
		(x)->ks_kid, (x)->ks_module, (x)->ks_instance, (x)->ks_name, \
		(x)->ks_data_size, (x)->ks_ndata,(x)->ks_type);

#define KS_PRINT_DATA(x) \
	fprintf(stderr, "%s has type %d\n", (x)->name, (x)->data_type);

/**
 * @brief Update the instances described by the given ks->info. Does nothing if
 * the ID of the given chain has not been changed since the last update.
 * @param kc	The kstat chain to lookup and update the instances
 * 	of and described via `ks`.
 * @param ks	the target for lookup and update.
 * @return The number of instances found/updated, attached to the given `ks`.
 */
int update_instance(kstat_ctl_t *kc, ks_info_t *ks);

/**
 * @brief If a kstat_read() fails with `EAGAIN` wait that many milliseconds
 * 	before trying to read again.
 */
#define KS_READ_WAIT	10

/**
 * A wrapper around kstat_read(), which automatically retries reading stats if
 * it failes with an `EAGAIN` error.
 * @param the kstat chain owning the given instance.
 * @param ksp	The instance to read.
 * @param now	The current time as delivered by gethrtime(). If the delta to
 * 	the snaptime of the given instance is < 1s, read will be skipped and
 * 	ksp returned as is.
 * @param data	If provided ksp data are not of type `KSTAT_TYPE_NAMED` or
 * 	`KSTAT_TYPE_TIMER` the `kstat_data_lookup()` will not work. Use this buffer
 * 	to get a copy of the raw the data.
 * @return NULL on error, ksp otherwise.
 */
kstat_t *ks_read(kstat_ctl_t *kc, kstat_t *ksp, hrtime_t now, void *data);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_KSUTIL_H
