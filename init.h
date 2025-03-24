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
 * @file init.h
 * Utilities to initialize node related stuff.
 */

#ifndef SOLMEX_INIT_H
#define SOLMEX_INIT_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the node stack.
 * @param cfg	The configuration to use.
 * @param compact	If \c true, no HELP/TYPE comments get generated and thus
 *	will not be emitted in a client response.
 * @param tasks	Set to the number of tasks which need to be queried on
 *	client requests.
 * @return \c NULL on error, the sensor list otherwise.
 */
void start(node_cfg_t *cfg, bool compact, uint32_t *tasks);

/**
 * @brief Shutdown the node stack and cleanup any allocated resources (and
 *	prepare for exit).
 * @param list	The list of sensors to release.
 * @return \c 0 on success, a number > 0 otherwise.
 */
void stop(void);

char *getVersions(psb_t *report, bool compact);

#ifdef __cplusplus
}
#endif

#endif	// SOLMEX_INIT_H
