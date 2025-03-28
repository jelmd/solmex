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
 * @file cpuinfo.h
 * Collect cpu info via kstats.
 */

#ifndef SOLMEX_CPUINFO_H
#define SOLMEX_CPUINFO_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void collect_cpuinfo(psb_t *sb, bool compact);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_CPUINFO_H
