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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <libprom/prom.h>

#include "mib.h"
#include "mib_impl.h"
#include "ks_util.h"

typedef enum ks_info_idx {
	KS_IDX_RAWIP,
	KS_IDX_IP,
	KS_IDX_ICMP,
	KS_IDX_UDP,
	KS_IDX_TCP,
	KS_IDX_SCTP,
	KS_IDX_MAX,			// last entry by cotract
} ks_info_idx_t;

typedef enum mib_stat_mode {
	MIB_NONE = 0,			/** Skip collecting metrics for mib2 class */
	MIB_NORMAL = 1,			/**< Provide basic metrics. */
	MIB_EXTENDED = 1 << 1,	/**< Provide extended metrics, i.e. more details. */
	MIB_ALL	= 1 << 2,		/**< Emitt all metrics provided by the related kstat instances */
} mib_stat_mode_t;
#define MIB_MODE_SHIFT 4	/** Waste a little bit - 6*4 = 24 - still enough space */
#define MIB_MODE_MASK 0x0000000f
#define MIB_MODE_ANY  (0xffffffff >> (32 - KS_IDX_MAX * MIB_MODE_SHIFT))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	{ NULL, 0, "rawip", -1, 0, NULL },
	{ NULL, 0, "ip", -1, 0, NULL },
	{ NULL, 0, "icmp", -1, 0, NULL },
	{ NULL, 0, "udp", -1, 0, NULL },
	{ NULL, 0, "tcp", -1, 0, NULL },
	{ NULL, 0, "sctp", -1, 0, NULL },
};
#pragma GCC diagnostic pop

static const char **aknames[] = {
	knames_rawip, knames_ip, knames_icmp, knames_udp, knames_tcp, knames_sctp
};
static const char **asnames[] = {
	snames_rawip, snames_ip, snames_icmp, snames_udp, snames_tcp, snames_sctp
};
static const char **asdescs[] = {
	sdesc_rawip, sdesc_ip, sdesc_icmp, sdesc_udp, sdesc_tcp, sdesc_sctp
};

static uint32_t *astats[] = {
	rawipstats_r, ipstats_r, icmpstats_r, udpstats_r, tcpstats_r, sctpstats_r,
	rawipstats_x, ipstats_x, icmpstats_x, udpstats_x, tcpstats_x, sctpstats_x,
	rawipstats_a, ipstats_a, icmpstats_a, udpstats_a, tcpstats_a, sctpstats_a
};

// C does not allow to use 'const ...' as initializers
static uint32_t astats_sz[] = {
	ARRAY_SIZE(rawipstats_r), ARRAY_SIZE(ipstats_r), ARRAY_SIZE(icmpstats_r),
		ARRAY_SIZE(udpstats_r), ARRAY_SIZE(tcpstats_r), ARRAY_SIZE(sctpstats_r),
	ARRAY_SIZE(rawipstats_x), ARRAY_SIZE(ipstats_x), ARRAY_SIZE(icmpstats_x),
		ARRAY_SIZE(udpstats_x), ARRAY_SIZE(tcpstats_x), ARRAY_SIZE(sctpstats_x),
	ARRAY_SIZE(rawipstats_a), ARRAY_SIZE(ipstats_a), ARRAY_SIZE(icmpstats_a),
		ARRAY_SIZE(udpstats_a), ARRAY_SIZE(tcpstats_a), ARRAY_SIZE(sctpstats_a)
};

#define SET_MIB_MODE(idx, val) \
	mode &= ~(MIB_MODE_MASK << (MIB_MODE_SHIFT * (idx))); \
	mode |= ((val) << (MIB_MODE_SHIFT * (idx)));
#define GET_MIB_MODE(idx) \
	((mode >> (MIB_MODE_SHIFT * (idx))) & MIB_MODE_MASK)

mib_mods_t
parse_mib_mode_list(const char *mode_str) {
	char buf[_POSIX_ARG_MAX], *t;
	size_t len;
	mib_mods_t mode = 0, current_mode = MIB_EXTENDED;

	if (mode_str == NULL)
		return mode;

	if ((len = strlen(mode_str)) > (_POSIX_ARG_MAX - 2)) {
		fprintf(stderr, "mib2 mode string to parse is too long (%ld).\n", len);
		return MIB_MODE_FAIL;
	}
	if (len == 0)
		return (MIB_NONE << (KS_IDX_RAWIP * MIB_MODE_SHIFT))
		| (MIB_EXTENDED << (KS_IDX_IP * MIB_MODE_SHIFT))
		| (MIB_EXTENDED << (KS_IDX_ICMP * MIB_MODE_SHIFT))
		| (MIB_NORMAL << (KS_IDX_UDP * MIB_MODE_SHIFT))
		| (MIB_NORMAL << (KS_IDX_TCP * MIB_MODE_SHIFT))
		| (MIB_NONE << (KS_IDX_SCTP * MIB_MODE_SHIFT));

	strcpy(buf, mode_str);
	buf[len] = ',';
	buf[len + 1] = '\0';
	t = buf;

	while (*t) {
		char *s = strchr(t, ',');
		if (s == NULL)
			break;
		s[0] = '\0';
		if ((s - t) == 1) {
			char c = t[0];
			if (c == '0' || c == 'n') {
				current_mode = MIB_NONE;
			} else if (c == '1' || c == 'r') {
				current_mode = MIB_EXTENDED;
			} else if (c == '1' || c == 'x') {
				current_mode = MIB_EXTENDED;
			} else if (c == '2' || c == 'a') {
				current_mode = MIB_ALL;
			} else {
				fprintf(stderr, "Unknown mib mode '%c'.\n", c);
				return MIB_MODE_FAIL;
			}
		} else if (strcmp(t, "none") == 0) {
			current_mode = MIB_NONE;
		} else if ((strcmp(t, "normal") == 0) || (strcmp(t, "regular") == 0)) {
			current_mode = MIB_NORMAL;
		} else if (strcmp(t, "extended") == 0) {
			current_mode = MIB_EXTENDED;
		} else if (strcmp(t, "all") == 0) {
			current_mode = MIB_ALL;
		} else if (strcmp(t, "rawip") == 0) {
			SET_MIB_MODE(KS_IDX_RAWIP, current_mode)
		} else if (strcmp(t, "ip") == 0) {
			SET_MIB_MODE(KS_IDX_IP, current_mode)
		} else if (strcmp(t, "icmp") == 0) {
			SET_MIB_MODE(KS_IDX_ICMP, current_mode)
		} else if (strcmp(t, "udp") == 0) {
			SET_MIB_MODE(KS_IDX_UDP, current_mode)
		} else if (strcmp(t, "tcp") == 0) {
			SET_MIB_MODE(KS_IDX_TCP, current_mode)
		} else if (strcmp(t, "sctp") == 0) {
			SET_MIB_MODE(KS_IDX_SCTP, current_mode)
		} else {
			fprintf(stderr, "Unknown mib mode '%s'.\n", t);
			return MIB_MODE_FAIL;
		}
		t = s + 1;
	}
	return mode;
}

void
collect_mib(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now, mib_mods_t mode) {
	kstat_t *ksp;
	kstat_named_t *knp;
	char buf[32];
	ks_info_idx_t kidx;

	size_t psz = 0;

	bool free_sb = sb == NULL;

	if (mode == MIB_MODE_FAIL || (mode & MIB_MODE_ANY) == 0)
		return;
	mode &= MIB_MODE_ANY;

	PROM_DEBUG("collect_mibstat (0x%x)...", mode);
	if (free_sb) {
		sb = psb_new();
		if (sb == NULL) {
			PROM_WARN("collect_mib: %s", strerror(errno));
			return;
		}
	}
	psz = psb_len(sb);

	for (kidx = 0; kidx < KS_IDX_MAX; kidx++) {
		const char **knames, **snames, **sdescs;
		uint32_t stats_sz, m;
		uint32_t *stats;
		int i, n;
		uint8_t type = GET_MIB_MODE(kidx);
		if (type == MIB_NONE)
			continue;

		n = update_instance(kc, &kstat[kidx]);
		if (n < 1)
			continue;

		type >>= 1;	// adjust for none
		knames = aknames[kidx];
		snames = asnames[kidx];
		sdescs = asdescs[kidx];
		stats_sz = astats_sz[KS_IDX_MAX * type + kidx];
		stats = astats[KS_IDX_MAX * type + kidx];

		for (m = 0; m < stats_sz; m++) {
			uint32_t l = stats[m];
			if (!compact)
				addPromInfo4("", snames[l],
					(kidx == KS_IDX_TCP && l == TCP_IDX_CURRESTAB) ||
					(kidx == KS_IDX_SCTP && l == SCTP_IDX_SCTPCURRESTAB)
						? "gauge" : "counter", sdescs[l]);
			for (i = 0; i < n; i++) {
				if ((ksp = ks_read(kc, kstat[kidx].ksp[i], now, NULL)) != NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
				if ((knp = kstat_data_lookup(ksp, knames[l])) != NULL) {
					if (knp->data_type == KSTAT_DATA_UINT32) {
						sprintf(buf, " %u\n", knp->value.ui32);
					} else if (knp->data_type == KSTAT_DATA_UINT64) {
						sprintf(buf, " %ld\n", knp->value.ui64);
					} else if (knp->data_type == KSTAT_DATA_INT32) {
						sprintf(buf, " %d\n", knp->value.i32);
					} else if (knp->data_type == KSTAT_DATA_INT64) {
						// only for sctp
						sprintf(buf, " %ld\n", knp->value.i64);
					} else {
						PROM_WARN("Software bug: unsupported KSTAT type %d for %s",
							knp->data_type, snames[l]);
						continue;
					}
					psb_add_str(sb, snames[l]);
					psb_add_str(sb, buf);
				}
#pragma GCC diagnostic pop
				}
			}
		}
	}

	if (free_sb) {
		if (psb_len(sb) != psz)
			fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_mibstat done", "");
}
