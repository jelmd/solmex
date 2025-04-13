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
 * @file network.h
 * Collect network interface data via kstats.
 */

#ifndef SOLMEX_NETWORK_H
#define SOLMEX_NETWORK_H

#include <kstat.h>
#include <sys/dls_mgmt.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nic_stat_quantity {
	NICSTAT_NONE = 0,
	NICSTAT_NORMAL,		/**< Just basic metrics usually sufficient in day-by-day work */
	NICSTAT_EXTENDED,	/**< More details wrt. security related package counters */
	NICSTAT_ALL			/**< Emitt all metrics provided by the related kstat instance */
} nic_stat_quantity_t;

typedef enum nic_filter_flag {
	NICFILTER_PHYS = DATALINK_CLASS_PHYS,
	NICFILTER_VNIC = DATALINK_CLASS_VNIC,
	NICFILTER_INCL = 1 << 24,
	NICFILTER_EXCL = 1 << 25,
} nic_filter_flag_t;

#define NICFILTER_OP_MASK   0x00F00000
#define NICFILTER_TYPE_MASK 0x0000FFFF

typedef struct nic_filter {
	uint32_t flags;		//  DATALINK_CLASS_{PHYS | VNIC} | INCL | EXCL
	regex_t *regex;
} nic_filter_t;

typedef struct nic_filter_chain {
	nic_filter_t *filter;
	uint32_t pos;
	uint32_t sz;
} nic_filter_chain_t;

/**
 * @brief Parse the given filter string and append the extracted filters to
 * 	the given list.
 * @param s	The string to parse.
 * @param list	Where to append the extracted filters. If NULL, a new one gets
 * 	created.
 * @return 0 on success, a value != 0 otherwise.
 */
int parse_nic_filter(char *s, nic_filter_chain_t **list);

/**
 * @brief Get data for various network interfaces.
 * @param sb	where to add the stats.
 * @param compact	whether to add HELP and TYPE comments
 * @param kc	The kstat chain to use.
 * @param now	The current time as delivered by gethrtime().
 * @param ntype Quantity of metrics to emit.
 * @param nfc	NIC filter chain.
 */
void collect_nicstat(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now,
	nic_stat_quantity_t ntype, nic_filter_chain_t *nfc);

#ifdef __cplusplus
}
#endif

#endif  // SOLMEX_NETWORK_H
