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
 * @file mib.h
 * kstat -c mibs related defintions/functions/etc.
 */
#ifndef SOLMEX_MIB_H
#define SOLMEX_MIB_H

#include <kstat.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t mib_mods_t;
#define MIB_MODE_FAIL 0xffffffff
#define MIB_MODE_NONE 0

/**
 * @brief Parse the given mib mode list string and return the corresponding mode mask.
 * @param s	The string to parse. An empty string is equal to the default mode.
 * @return The resulting mode mask, `MIB_MODE_FAIL` on error.
 */
mib_mods_t parse_mib_mode_list(const char *s);

/**
 * @brief Get metrics of the mib2 kstat class for the running zone.
 * @param sb    where to add the stats.
 * @param compact   whether to add HELP and TYPE comments
 * @param kc    The kstat chain to use.
 * @param now   The current time as delivered by gethrtime().
 * @param mode	A bit set of `mib_stat_mode_t` regarding the metrics to emit.
 */
void collect_mib(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now, mib_mods_t mode);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_MIB_H
