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

/** NOTE: The kernel updates these values usuallly once per second, only! */

/**
 * @brief Get the average load statistics for the whole system for the last 1, 5,
 * and 15 minutes (via unix::system_misc).
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 */
void collect_load(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now);

/**
 * @brief Get the number of cpus online and offline (via unix::pset).
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param now	The current time as delivered by gethrtime().
 */
void collect_cpu_state(psb_t *sb, bool compact, hrtime_t now);

/**
 * @brief Get the value of the system's run, swap and wait queue counter (via
 * 	unix::sysinfo)
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 */
void collect_procq(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now);

/**
 * @brief Get swap related kernel stats (via unix::vminfo).
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 */
void collect_swap(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now);

/**
 * @brief Get the value of some static kernel vars like page size and ticks per
 * 	second.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 */
void collect_units(psb_t *sb, bool compact);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_LOAD_H
