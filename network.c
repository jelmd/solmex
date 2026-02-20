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
#include <kstat.h>
#include <sys/loadavg.h>
#include <rpcsvc/rstat.h>
#include <libdllink.h>
#include <zone.h>
#include <sys/utsname.h>
#include <errno.h>
#include <string.h>

#include <libprom/prom.h>

#include "network.h"
#include "network_impl.h"
#include "ks_util.h"

typedef enum ks_info_idx {
	KS_IDX_NICMOD = 0,
	KS_IDX_LNKMOD,
	KS_IDX_MAX,			// last entry by cotract
} ks_info_idx_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	// NOTE: zoned NICS have usually a 2nd instance != 0 but with the same
	// module alias linkname! They provide the same stats, so let's ignore all != 0.
	{ NULL, 0, "link", -1, 0, NULL },
	{ "link", 0, NULL, -1, 0, NULL },	// illumos fallback
};
#pragma GCC diagnostic pop

#define VNIC "vnic"
#define PHYS "phys"

typedef struct nic {
	datalink_id_t lid;
	char name[MAXLINKNAMELEN];
	datalink_class_t class;
	const char *tname;
	zoneid_t zid;
} nic_t;

typedef struct nic_bucket {
	int len;
	int sz;
	nic_t *nic;
} nic_bucket_t;

#define EXTENT 8
#define NEXT_NIC(bp)	((bp)->nic[(bp)->len])

static int
record_link(dladm_handle_t dh, datalink_id_t linkid, void *arg) {
	nic_bucket_t *bucket = arg;
	dladm_status_t status;
	uint32_t flags;
	uint32_t media;

	if (bucket == NULL)
		return DLADM_STATUS_BADARG;

	if (bucket->len == bucket->sz) {
		nic_t *rnic = realloc(bucket->nic, (bucket->sz + EXTENT) * sizeof(nic_t));
		if (rnic == NULL) {
			PROM_WARN("Unable to allocate NIC buffer. Link %u ignored.", linkid);
			return DLADM_STATUS_NOMEM;
		}
		bucket->nic = rnic;
		bucket->sz += EXTENT;
	}
	// lets populate it here (instead of just recording the linkId) immediately
	// so we do not clutter other places with dladm ...
#ifdef __illumos__
	status = dladm_datalink_id2info(dh, linkid, &flags,
		&(NEXT_NIC(bucket).class),
		&media, NEXT_NIC(bucket).name, MAXLINKNAMELEN);
#else
	status = dladm_datalink_id2linkinfo(dh, linkid, &flags,
		&(NEXT_NIC(bucket).class),
		&media, NEXT_NIC(bucket).name, MAXLINKNAMELEN, &(NEXT_NIC(bucket).zid));
#endif
	if (status == DLADM_STATUS_OK) {
#ifdef __illumos__
		zoneid_t zid = ALL_ZONES;
		status = zone_check_datalink(&zid, linkid);
		NEXT_NIC(bucket).zid = status == 0 ? zid : 0;
#endif
		NEXT_NIC(bucket).lid = linkid;
		if (NEXT_NIC(bucket).class == DATALINK_CLASS_PHYS) {
			NEXT_NIC(bucket).tname = PHYS;
			bucket->len++;
		} else if (NEXT_NIC(bucket).class == DATALINK_CLASS_VNIC) {
			NEXT_NIC(bucket).tname = VNIC;
			bucket->len++;
		} else {
			// should not happen because we restrict the walker accordingly
			char cbuf[DLADM_STRSIZE];
			(void) dladm_class2str(NEXT_NIC(bucket).class, cbuf);
			PROM_WARN("Interface type '%s' for '%s' is not supported - skipping.",
				cbuf, NEXT_NIC(bucket).name);
		}
	}
	return DLADM_WALK_CONTINUE;
}

#define NIC_BUCKET_INIT_SZ 8
#define LOOKUP_CLASS_TYPES	DATALINK_CLASS_PHYS | DATALINK_CLASS_VNIC	// DATALINK_CLASS_ALL
#define LOOKUP_MEDIA_TYPES	DL_ETHER									// DATALINK_ANY_MEDIATYPE
#define LOOKUP_FLAGS		DLADM_OPT_ACTIVE	// if not active, ignore it

static nic_bucket_t *
collectNicInfo(void) {
	static dladm_handle_t dladm = NULL;
	dladm_status_t status;
	static nic_bucket_t *bucket = NULL;

	if (dladm == NULL) {
#ifdef __illumos__
		if ((status = dladm_open(&dladm)) != DLADM_STATUS_OK) {
#else
		if ((status = dladm_open(&dladm, NULL)) != DLADM_STATUS_OK) {
#endif
			PROM_WARN("Could not open /dev/dld", "");
			return bucket;
		}
	}
	if (bucket == NULL) {
		bucket = calloc(1, sizeof(nic_bucket_t));
		if (bucket == NULL) {
			PROM_WARN("Unable to allocate nic bucket: ", strerror(errno));
			return NULL;
		}
	}
	bucket->len = 0;
	dladm_walk_datalink_id(record_link, dladm, bucket,
		LOOKUP_CLASS_TYPES, LOOKUP_MEDIA_TYPES, LOOKUP_FLAGS);

	return bucket;
}

#define ATTR_NICNAME "nic"		// node-exporter uses: "device" instead
#define ATTR_GZ "gz"
#define ATTR_NGZ "ngz"
#define ATTR_TYPE "type"

static char **
updateMetricAttrs(char **metric_attr, int *metric_attr_sz, int n,
	ks_info_idx_t idx, nic_filter_chain_t *nfc)
{
	int i, k, r;
	char *gz = NULL;
	char zname[ZONENAME_MAX];
	psb_t *s = psb_new();
	uint64_t *nicset = NULL, nicset_sz = 0, f;
	char **nics = NULL;

	nic_bucket_t *nb = collectNicInfo();

	if (nb == NULL)
		return metric_attr;

	for (i = 0; i < *metric_attr_sz; i++) {
		free(metric_attr[i]);
		metric_attr[i] = NULL;
	}
	if (n > *metric_attr_sz) {
		char **t = realloc(metric_attr, n * sizeof(char *));
		if (t == NULL) {
			PROM_WARN("Unable to allocate nicstat metrics - skipping.", "");
			free(metric_attr);
			metric_attr = NULL;
			*metric_attr_sz = 0;
			return metric_attr;
		}
		metric_attr = t;
		*metric_attr_sz = n;
	}
	memset(metric_attr, 0, n * sizeof(char *));	// cheap, so lets reset for now
	nics = calloc(n, sizeof (char *));
	if (nics == NULL) {
		PROM_WARN("Unable to allocate nicstat metrics - skipping.", "");
		goto end;
	}
	for (i = 0; i < n; i++) {
		nics[i] = idx == KS_IDX_NICMOD
			? kstat[idx].ksp[i]->ks_module
			: kstat[idx].ksp[i]->ks_name;
	}

	if (nfc != NULL && nfc->pos > 0) {
		uint64_t offset;
		nicset_sz = (n >> 6) + 1;
		nicset = calloc(nicset_sz, sizeof(uint64_t));
		if (nicset == NULL) {
			PROM_WARN("Unable to allocate nicstat metrics - skipping.", "");
			goto end;
		}
		if (nfc->filter[0].flags & NICFILTER_EXCL) {
			for (offset = 0; offset < nicset_sz; offset++)
				nicset[offset] = 0xFFFFFFFFFFFFFFFF;
		}
		for (f = 0; f < nfc->pos; f++) {
			for (i = 0, r = 0, offset = 0; i < nb->len; i++, r++) {
				if (r == 64) {
					offset++;
					r = 0;
				}
				if ((nb->nic[i].class & nfc->filter[f].flags) == 0)
					continue;
				if (regexec(nfc->filter[f].regex, nb->nic[i].name, 0, NULL, 0))
					continue;
				if (nfc->filter[f].flags & NICFILTER_INCL) {
					nicset[offset] |= (1UL << r);
				} else {
					nicset[offset] &= ~(1UL << r);
				}
			}
		}
		for (i = 0, r = 0, offset = 0; i < nb->len; i++, r++) {
			if (r == 64) {
				offset++;
				r = 0;
			}
			if ((nicset[offset] & (1UL << r)) == 0) {
				psb_add_str(s, nb->nic[i].name);
				psb_add_str(s, ", ");
				nb->nic[i].name[0] = '\0'; // so no match occurs below
			}
		}
		f = psb_len(s);
		if (f > 0) {
			psb_truncate(s, f - 2);
			psb_add_char(s, '.');
			PROM_INFO("Excluding NIC metrics for: %s", psb_str(s));
		}
	}

	for (i = 0; i < n; i++) {
		size_t l;

		psb_truncate(s, 0);
		for (k = 0; k < nb->len; k++) {
			if (strcmp(nb->nic[k].name, nics[i]) != 0)
				continue;
			psb_add_str(s, "{" ATTR_NICNAME "=\"");
			psb_add_str(s, nics[i]);
			psb_add_str(s, "\"," ATTR_TYPE "=\"");
			psb_add_str(s, nb->nic[k].tname);
			psb_add_str(s, "\",");
			if (gz == NULL) {
				struct utsname uts;
				gz = (uname(&uts) != -1) ? strdup(uts.nodename) : strdup("");
			}
			psb_add_str(s, ATTR_GZ "=\"");
			psb_add_str(s, gz);
			psb_add_str(s, "\",");
			if (nb->nic[k].zid != 0) {
				if (getzonenamebyid(nb->nic[k].zid, zname, ZONENAME_MAX) == -1) {
					char *err = strerror(errno);
					sprintf(zname, "link%u", nb->nic[k].lid);
					PROM_WARN("Unbale to get zonename for Id %d: %s "
						"(using '%s' instead)", nb->nic[k].zid, err, zname);
				}
				psb_add_str(s, ATTR_NGZ "=\"");
				psb_add_str(s, zname);
				psb_add_str(s, "\",");
			}
			break;
		}
		if ((l = psb_len(s)) != 0) {
			psb_truncate(s, l - 1);
			psb_add_str(s, "} ");
			metric_attr[i] = psb_dump(s);
		} else {
			metric_attr[i] = NULL;
		}
	}

end:
	free(gz);
	free(nicset);
	free(nics);
	psb_destroy(s);
	return metric_attr;
}

static regex_t *
get_regex(int *res, char *regex, const char *target) {
	*res = 0;
	if (regex == NULL)
		return NULL;

	regex_t *r = malloc(sizeof(regex_t));
	if (r == NULL) {
		perror(target);
		*res = 1;
		return NULL;
	}

	*res = regcomp(r, regex, REG_EXTENDED|REG_NOSUB);
	if (*res == 0)
		return r;

	size_t sz = regerror(*res, r, (char *)NULL, (size_t)0);
	char *s = malloc(sz);
	if (s == NULL) {
		perror(target);
		free(r);
		*res = 1;
		return NULL;
	}

	regerror(*res, r, s, sz);
	fprintf(stderr, "Unable to compile regex for %s%s", target, s);
	free(s);
	free(r);
	// since state of preg is undefined, we prefer to not call regfree() here.
	*res = 1;
	return NULL;
}

#define NIC_FILTER_EXTENT 8

static int
addNicFilter(nic_filter_chain_t **list, regex_t *regex, uint32_t flags) {
	if (*list == NULL) {
		*list = calloc(1, sizeof(nic_filter_chain_t));
		if (*list == NULL) {
			perror("Not enough memory for new nic filter chain: ");
			return 0;
		}
	}
	if ((*list)->pos == (*list)->sz) {
		nic_filter_t *f =
			realloc((*list)->filter, NIC_FILTER_EXTENT * sizeof(nic_filter_t));
		if (f == NULL) {
			perror("Not enough memory for new nic filter: ");
			return 0;
		}
		(*list)->filter = f;
		(*list)->sz += NIC_FILTER_EXTENT;
	}
	(*list)->filter[(*list)->pos].flags = flags;
	(*list)->filter[(*list)->pos].regex = regex;
	(*list)->pos++;
	return 1;
}

int
parse_nic_filter(char *s, nic_filter_chain_t **list) {
	char *t, c, *i, *s2;
	int res;
	uint32_t flags, last_op = 0;
	size_t len;

	if (s == NULL || strlen(s) == 0)
		return 0;

	len = strlen(s);
	s2 = malloc(len + 2);
	if (s2 == NULL) {
		fprintf(stderr, "Unable to copy string to parse: %s", strerror(errno));
		return 1;
	}
	strcpy(s2, s);
	s2[len] = ',';
	s2[len+1] = '\0';
	i = s2;

	while ((t = strchr(i, ',')) != NULL) {
		*t = '\0';
		flags = 0;
		if (i[1] == ':') {
			c = i[0];
			if (c < 'a') {
				flags = NICFILTER_EXCL;
				c += 32;
			} else {
				flags = NICFILTER_INCL;
			}
			if (c == 'a') {
				flags |= NICFILTER_PHYS | NICFILTER_VNIC;
			} else if (c == 'p') {
				flags |= DATALINK_CLASS_PHYS;
			} else if (c == 'v') {
				flags |= DATALINK_CLASS_VNIC;
			} else {
				fprintf(stderr, "Unknown include/exclude operator '%c' in '%s'",
					c, t);
				free(s2);
				return 2;
			}
			i += 2;
		}
		regex_t *r = get_regex(&res, i, "nic filter: ");
		if (r == NULL) {
			free(s2);
			return 3;
		}
		if (flags == 0) {
			if (last_op == 0) {
				flags = (*list == NULL || (*list)->pos == 0)
					? (NICFILTER_INCL | NICFILTER_PHYS | NICFILTER_VNIC)
					: (*list)->filter[(*list)->pos - 1].flags;
			} else {
				flags = last_op;
			}
		}
		last_op = flags;
		if (addNicFilter(list, r, flags) == 0) {
			free(s2);
			return 4;
		}
		i = t + 1;
	}
	free(s2);
	return 0;
}

static char *
updateSpeed(kstat_ctl_t *kc, ks_info_idx_t ks_idx, int n, char **metric_attr,
	bool compact, hrtime_t now)
{
	kstat_t *ksp;
	kstat_named_t *knp;
	psb_t *sb = psb_new();
	char *res = NULL;
	char buf[32];

	PROM_INFO("Checking speed for %d links.", n);
	if (!compact)
		addPromInfo4("", snames[NET_IDX_IFSPEED_BPS],
			"gauge", sdesc[NET_IDX_IFSPEED_BPS]);

	if (ks_idx == KS_IDX_LNKMOD) {
		if (n < 1)
			goto noSpeed;

		for (int i = 0; i < n; i++) {
			if (metric_attr[i] == NULL)
				continue;

			if ((ksp = ks_read(kc, kstat[KS_IDX_LNKMOD].ksp[i], now, NULL)) != NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
				knp = kstat_data_lookup(kstat[KS_IDX_LNKMOD].ksp[i],
					knames[NET_IDX_IFSPEED_BPS]);
#pragma GCC diagnostic pop
				if (knp != NULL) {
					psb_add_str(sb, snames[NET_IDX_IFSPEED_BPS]);
					psb_add_str(sb, metric_attr[i]);
					sprintf(buf, " %ld\n", knp->value.ui64);
					psb_add_str(sb, buf);
				}
			}
		}
	} else {
		// need to lookup ::${nicname}:ifspeed - unfortunately we cant rely on
		// the order wrt. nicname
		kstat[KS_IDX_LNKMOD].last_kid = 0;	// force an update
		int n2 = update_instance(kc, &kstat[KS_IDX_LNKMOD]);
		if (n2 < 1)
			goto noSpeed;

		for (int i = 0; i < n; i++) {
			if (metric_attr[i] == NULL)
				continue;

			for (int k = 0; k < n2; k++) {
				if (strcmp(kstat[KS_IDX_NICMOD].ksp[i]->ks_module,
					kstat[KS_IDX_LNKMOD].ksp[k]->ks_name) != 0)
					continue;

				if ((ksp = ks_read(kc, kstat[KS_IDX_LNKMOD].ksp[k], now, NULL)) != NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
					knp = kstat_data_lookup(kstat[KS_IDX_LNKMOD].ksp[k],
						knames[NET_IDX_IFSPEED_BPS]);
#pragma GCC diagnostic pop
					if (knp != NULL) {
						psb_add_str(sb, snames[NET_IDX_IFSPEED_BPS]);
						psb_add_str(sb, metric_attr[i]);
						sprintf(buf, " %ld\n", knp->value.ui64);
						psb_add_str(sb, buf);
						break;
					}
				}
			}
		}
	}
	res = psb_dump(sb);
noSpeed:
	psb_destroy(sb);
	return res;
}

void
collect_nicstat(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now,
	nic_stat_quantity_t ntype, nic_filter_chain_t *nfc)
{
	kstat_t *ksp;
	kstat_named_t *knp;
	char buf[32];

	static int metric_attr_sz = 0;
	static char **metric_attr = NULL;
	static ks_info_idx_t ks_idx = KS_IDX_NICMOD;
	int i, n, m, stats_sz;
	net_idx_t *stats;
	static char *speed = NULL;

	bool nicmode;

	PROM_DEBUG("collect_nicstat ...", "");
	if (ntype == NICSTAT_NONE)
		return;

	// if a vnic gets created|deleted or the interface comes up|goes down
	// e.g. via "ipadm create-addr ..." or "ipadm {up,down}-addr ..." the
	// chain ID gets changed.
	bool check = kc->kc_chain_id != kstat[ks_idx].last_kid;

	n = update_instance(kc, &kstat[ks_idx]);
	if (n < 1) {
		if (ks_idx != KS_IDX_NICMOD)
			return;
		ks_idx = KS_IDX_LNKMOD;		// use fallback
		n = update_instance(kc, &kstat[ks_idx]);
		if (n < 1)
			return;
	}

	if (check || n > metric_attr_sz) {
		metric_attr =
			updateMetricAttrs(metric_attr, &metric_attr_sz, n, ks_idx, nfc);
		if (n > metric_attr_sz) {
			PROM_WARN("Skipping nicstat metrics", "");
			return;
		}
		free(speed);
		speed = updateSpeed(kc, ks_idx, n, metric_attr, compact, now);
	}

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	psb_add_str(sb, speed);

	nicmode = ks_idx == KS_IDX_NICMOD;
	if (nicmode) {
		if (ntype == NICSTAT_NORMAL) {
			stats = rnicstats;
			stats_sz = rnicstats_sz;
		} else if (ntype == NICSTAT_EXTENDED) {
			stats = xnicstats;
			stats_sz = xnicstats_sz;
		} else {
			stats = anicstats;
			stats_sz = anicstats_sz;
		}
	} else {
		if (ntype == NICSTAT_NORMAL) {
			stats = rlnkstats;
			stats_sz = rlnkstats_sz;
		} else if (ntype == NICSTAT_EXTENDED) {
			stats = xlnkstats;
			stats_sz = xlnkstats_sz;
		} else {
			stats = alnkstats;
			stats_sz = alnkstats_sz;
		}
	}

	for (m = 0; m < stats_sz; m++) {
		net_idx_t l = stats[m];
		if (!compact) {
			addPromInfo4("", snames[l],
				(l == NET_IDX_LINK_STATE || l == NET_IDX_PHYS_STATE)
					? "gauge"
					: "counter",
				sdesc[l]);
		}
		for (i = 0; i < n; i++) {
			if (metric_attr[i] == NULL)
				continue;
			if ((ksp = ks_read(kc, kstat[ks_idx].ksp[i], now, NULL)) != NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
				if ((knp = kstat_data_lookup(ksp, knames[l])) != NULL) {
					psb_add_str(sb, snames[l]);
					psb_add_str(sb, metric_attr[i]);
					sprintf(buf, "%ld\n", knp->value.ui64);
					psb_add_str(sb, buf);
				}
#pragma GCC diagnostic pop
			}
		}
	}
	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_nicstat done", "");
}
