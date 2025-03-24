/*
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License") 1.1!
 * You may not use this file except in compliance with the License.
 *
 * See  https://spdx.org/licenses/CDDL-1.1.html  for the specific
 * language governing permissions and limitations under the License.
 *
 * Copyright 2021 Jens Elkner (jel+solmex-src@cs.ovgu.de)
 */
#include <stdio.h>
#include <string.h>

#include <prom_string_builder.h>

#include "common.h"
#include "prom_node.h"

void
collect_node(psb_t *sb, bool compact) {
	//uint8_t cc;
	//char buf[256];
	size_t sz = 0;
	bool free_sb = sb == NULL;

	if (free_sb) {
		sb = psb_new();
		if (sb == NULL) {
			perror("collect_dcmi: ");
			return;
		}
		sz = psb_len(sb);
	}

	if (!compact)
		printf("TBD\n");
	/*
	if (!compact)
		addPromInfo(SOLMEXM_NODE_STATNAME);
	sdr_power_t *p = get_stats(&cc);
	if (p == NULL || cc != 0)
		return;
	psb_add_str(sb, SOLMEXM_NODE_STATNAME_N "{value=\"now\"} ");
	sprintf(buf, "%d\n", p->curr);
	psb_add_str(sb, buf);
	*/

	if (free_sb) {
		if (psb_len(sb) != sz)
			fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
}
