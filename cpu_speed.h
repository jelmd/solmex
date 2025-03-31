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
 * @file cpu_speed.h
 * Collect CPU speeds via kstats.
 */

#ifndef SOLMEX_SPEED_H
#define SOLMEX_SPEED_H

#include <kstat.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the current and optionally the max frequency of all CPU strands
 * 	alias threads. Usually for day-by-day metrics this does not provide much
 * 	information and is just overhead. For day-by-day use the average load should
 *	be sufficient for general monitoring.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use. If NULL the values are obtained directly
 * 		from the kernel via syscall without making the indirection via the kstat
 *		machinery.
 * @param now	The current time as delivered by gethrtime().
 * @param include_max	Whether to include the max. frequencies of the CPU strands.
 * 		Usually it is the same as for the whole package alias chip alias socket
 * 		and already provided by the very cheap solmex_node_cpu_info metric.
 * 		So it should be false by default.
 */
void collect_cpu_speed(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now,
	bool include_max);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_SPEED_H
