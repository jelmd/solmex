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
 * @file fs.h
 * vopstats related defintions/functions/etc.
 */
#ifndef SOLMEX_FS_H
#define SOLMEX_FS_H

#include <kstat.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse the given fs list string and return the corresponding mode mask.
 * @param s	The string to parse. An empty string is equal to the default mode.
 * @param valid Gets set to @code 1 if the given string could be parsed
 * 	successfully, to @code 0 otherwise.
 * @return A reference to the config to be used in the collect_fs() call.
 */
void *parse_fs_mods_list(const char *s, int *valid);

/**
 * @brief Get metrics of the vopstats_ kstats.
 * @param sb    where to add the stats.
 * @param compact   whether to add HELP and TYPE comments
 * @param kc    The kstat chain to use.
 * @param now   The current time as delivered by gethrtime().
 * @param cfg	The reference returned by parse_fs_mods_list() or `NULL`.
 */
void collect_fs(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now, void *cfg);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_FS_H
