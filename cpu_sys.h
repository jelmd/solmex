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
 * @file cpu_sys.h
 * Collect cpu::sys stats via kstats.
 */

#ifndef SOLMEX_CPUSYS_H
#define SOLMEX_CPUSYS_H

#include <kstat.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cpu_sys_quantity {
	CPUSYS_NONE = 0,
	CPUSYS_NORMAL,		/**< Just basic metrics usually sufficient in day-by-day work */
	CPUSYS_EXTENDED,	/**< Add detailed metrics for exec, anon and file pages */
	//CPUSYS_ALL			/**< Emit all metrics provided by `kstat cpu::vm` */
} cpu_sys_quantity_t;

/**
 * @brief Collect cpu::vm stats.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 * @param mp	If \c true emit the stats for each CPU strand alias thread.
 * 		Otherwise the sum over all CPU strands alias threads gets emitted,
 *		only.
 * @param stype	The quantity of metrics to emit.
 */
void collect_cpusys(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now,
	bool mp, cpu_sys_quantity_t stype);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_CPUSYS_H
