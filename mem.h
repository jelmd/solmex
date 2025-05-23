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
 * @file mem.h
 * Collect system memory related metrics via kstats.
 */

#ifndef SOLMEX_MEM_H
#define SOLMEX_MEM_H

#include <kstat.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Collect unix:0:systempages stats.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 * @param details	If \c false just collect physmem, freemem, {min,lots}free,
 * 		and {fast,slow}scan. Otherwise add other metrics available in the
 * 		related kstat instance.
 */
void collect_sys_mem(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_MEM_H
