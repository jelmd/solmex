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
 * @file dmi.h
 * Collect DMI data via smbios.
 */

#ifndef SOLMEX_DMI_H
#define SOLMEX_DMI_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Collect DMI related metrics. For now they are all static, so once called it
 * will always append the same metric string to the given buffer.
 *
 * @param sb	where to append the metric results.
 * @param compact	whether to add HELP and TYPE comments.
 */
void collect_dmi(psb_t *sb, bool compact);

/**
 * Get the cache size in bytes of the CPU with the given number.
 *
 * NOTE: Since it causes lazy init of smbios related static metrics one should
 *   call collect_dmi(...) first, if it is intended to be used, too.
 *
 * @param cpuNum	The number of the CPU to lookup. CPUs get enumerated in the
 * 	same order as found via smbios(1M).
 */
int64_t get_cache_size(uint8_t cpuNum);

/**
 * Get the max speed in Hz of the CPU with the given number. On Solaris this is
 * usually the CPU internal value. It might be higher (a turbo speed) than what
 * 'kstat cpu_info:::supported_frequencies_Hz' reveals.
 *
 * NOTE: Since it causes lazy init of smbios related static metrics one should
 *   call collect_dmi(...) first, if it is intended to be used, too.
 *
 * @param cpuNum	The number of the CPU to lookup. CPUs get enumerated in the
 * 	same order as found via smbios(1M).
 */
int64_t get_turbo_speed(uint8_t cpuNum);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_DMI_H
