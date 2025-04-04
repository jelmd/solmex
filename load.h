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
 * @file load.h
 * Collect load averages and other non-network related info via kstats.
 */

#ifndef SOLMEX_LOAD_H
#define SOLMEX_LOAD_H

#include <kstat.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the average load statistics for the whole system for the last 1, 5,
 * and 15 minutes.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 */
void collect_load(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now);

/**
 * @brief Get the number of cpus online and offline.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 */
void collect_cpu_state(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now);

/**
 * @brief Get the value of the system's run, swap and wait queue counter.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 */
void collect_procq(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_LOAD_H
