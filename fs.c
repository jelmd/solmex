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
#include <sys/vnode.h>
#include <errno.h>
#include <zone.h>
#include <limits.h>

#include <libprom/prom.h>

#include "fs.h"
#include "ks_util.h"

typedef uint16_t fsmode_t;

typedef enum ks_info_idx {
	KS_IDX_UFS,			// obsolete
	KS_IDX_UVFS,		// fuse?
	KS_IDX_NFS,			// obsolete
	KS_IDX_NFS3,		// obsolete
	KS_IDX_AUTOFS,		// almost never shows something
	KS_IDX_MNTFS,		// not really interesting
	KS_IDX_LOFS,		// not really interesting
	KS_IDX_PROC,		// system managed- should be ok
	KS_IDX_TMPFS,		// usually does not need to be monitored
	KS_IDX_NFS4,
	KS_IDX_ZFS,
	KS_IDX_MAX,			// last entry by cotract
} ks_info_idx_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	{ NULL, -1, VOPSTATS_STR "ufs", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "uvfs", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "nfs", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "nfs3", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "autofs", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "mntfs", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "lofs", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "proc", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "tmpfs", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "nfs4", -1, 0, NULL },
	{ NULL, -1, VOPSTATS_STR "zfs", -1, 0, NULL }
};
#pragma GCC diagnostic pop

typedef uint32_t fs_mods_t;
#define FS_MODS_FAIL 0xffffffff
#define FS_MODS_NONE 0

#define SET_FS_MOD(var, idx)	(var) |= (1 << (idx))
#define GET_FS_MOD(var, idx) 	(((var) >> (idx)) & 0x1)

static zoneid_t MY_ZID = -1;
static char MY_ZNAME[ZONENAME_MAX];
#define ANY_ZID INT32_MAX

void
check_zone_vars(void) {
	if (MY_ZID != -1)
		return;

	MY_ZID = getzoneid();
	getzonenamebyid(MY_ZID, MY_ZNAME, ZONENAME_MAX);
}

typedef struct zinfo {
	char *zname;
	zoneid_t zid;
	fs_mods_t mods;
	struct zinfo *next;
} zinfo_t;

void
releaseZinfo(zinfo_t *head) {
	zinfo_t *zchain = head, *zi;
	while (zchain) {
		zi = zchain->next;
		free(zchain->zname);
		free(zchain);
		zchain = zi;
	}
	head = NULL;
}

// we expect very small lists, so a simple bubble sort should be ok
zinfo_t *
sortZinfos(zinfo_t *head) {
	bool swapped;
	zinfo_t **pzi, *last = NULL;

	if (head == NULL)
		return NULL;
	if (head->next == NULL)
		return head;

	do {
		swapped = false;
		pzi = &head;

		while ((*pzi)->next != last) {
			zoneid_t pzid = (*pzi)->zid;
			zoneid_t nzid = (*pzi)->next->zid;
			if (pzid == -1)
				pzid = ANY_ZID - 1;
			if (nzid == -1)
				nzid = ANY_ZID - 1;
			if (pzid > nzid ||
				(pzid == nzid && strcmp((*pzi)->zname, (*pzi)->next->zname) > 0))
			{
				zinfo_t *tmp = *pzi;
				swapped = true;
				*pzi = tmp->next;
				tmp->next = (*pzi)->next;
				(*pzi)->next = tmp;
			}
			pzi = &((*pzi)->next);
		}
		last = *pzi;
	} while (swapped);
	return head;
}

int
dump_cfg(zinfo_t *head, char *buf, size_t blen) {
	if (blen == 0)
		return 0;
	char nbuf[33];

	psb_t *b = psb_new();
	while(head) {
		bool comma = true;
		psb_add_str(b, head->zname);
		psb_add_char(b, '(');
		sprintf(nbuf, "%d):", head->zid);
		psb_add_str(b, nbuf);
		for (ks_info_idx_t i=FS_MODS_NONE; i < KS_IDX_MAX; i++) {
			if (GET_FS_MOD(head->mods, i)) {
				psb_add_str(b, kstat[i].name + strlen(VOPSTATS_STR));
				psb_add_char(b,',');
				comma = false;
			}
		}
		if (comma)
			psb_add_char(b, ',');
		head = head->next;
	}
	size_t len = psb_len(b);
	if (buf == NULL)
		return len;

	psb_truncate(b, len - 1);
	psb_truncate(b, blen);
	strcpy(buf, psb_dump(b));
	psb_destroy(b);
	return len;
}

void *
parse_fs_mods_list(const char *optarg) {
	if (optarg == NULL)
		return NULL;

	char *cfg, *t, *c, *s;
	psb_t *b = psb_new();
	size_t len = strlen(optarg);
	int zcount = 0;
	zinfo_t *zchain_head = NULL, *zchain, *zi = NULL, zdefault;
	bool ok = true, found;

	zdefault.mods = FS_MODS_NONE;
	SET_FS_MOD(zdefault.mods, KS_IDX_NFS4);
	SET_FS_MOD(zdefault.mods, KS_IDX_ZFS);
	zdefault.zname = strdup("any");
	zdefault.zid = ANY_ZID;

	check_zone_vars();
	if (len == 0) {
		psb_add_str(b, MY_ZNAME);
		psb_add_str(b,":,");
	} else {
		psb_add_str(b, optarg);
		psb_add_char(b, ',');
	}
	cfg = t = psb_dump(b);
	psb_truncate(b, 0);

	len = strlen(VOPSTATS_STR);
	while (*t) {
		s = strchr(t, ',');
		if (s == NULL)
			break;
		if (s == t + 1) {	// empty
			t += 2;
			continue;
		}
		s[0] = '\0';
		c = strchr(t, ':');
		if (c != NULL) {
			// handle zonename
			c[0] = '\0';
			zchain = zchain_head;
			if (strcmp(t, "this") == 0)
				t = MY_ZNAME;
			while (zchain) {
				if (strcmp(zchain->zname, t) == 0)
					break;
				zchain = zchain->next;
			}
			if (zchain == NULL) {
				zchain = malloc(sizeof(zinfo_t));
				if (zchain == NULL) {
					fprintf(stderr, "ERROR: Unable to create zinfo: %s\n", strerror(errno));
					ok = false;
					break;
				}
				zcount++;
				zchain->mods = FS_MODS_NONE;
				zchain->zname = strdup(t);
				zchain->zid = strcmp(t, "any") == 0 ? ANY_ZID : getzoneidbyname(t);
				if (zchain->zid == -1)
					fprintf(stderr, "WARNING: Zone '%s' not found!\n", t);
				zchain->next = zchain_head;
				zchain_head = zchain;
			}
			zi = zchain;
			if (c[1] == '\0' || c[1] == ',') {
				zchain->mods = zdefault.mods;
				if (c[1] == '\0')
					break;
				t = c + 2;
				continue;
			} else {
				t = c + 1;
			}
		}
		ks_info_idx_t k;
		found = false;
		if (strcmp(t, "none") == 0) {
			t = s+1;
			continue;
		}
		for (k = 0; k < KS_IDX_MAX; k++) {
			found = k == KS_IDX_PROC && strcmp(t, "procfs") == 0;
			if (found || strcmp(t, kstat[k].name + len) == 0) {
				found = true;
				if (zi == NULL) {
					if (t == cfg)
						zdefault.mods = FS_MODS_NONE;
					SET_FS_MOD(zdefault.mods, k);
				} else {
					SET_FS_MOD(zi->mods, k);
				}
				break;
			}
		}
		if (!found) {
			fprintf(stderr, "WARNING: Unknown fstype '%s' for %s zone ignored\n",
				t, (zi == NULL) ? "default" : zi->zname);
		}
		t = s+1;
	}
	// create the final array sorted by zid and put any at the end
	free(cfg);
	psb_destroy(b);
	if (!ok) {
		releaseZinfo(zchain_head);
		return NULL;
	}
	return sortZinfos(zchain_head);
}

// nefiles = ncreate + nmkdir + nsymlink
// namerm = nremove + nrmdir
// nnamechg = nrename + nlink + nsymlink
// nattrret = ngetattr + naccess + ngetsecattr + nfid
// nattrchg = nsetattr + nsetsecattr + nspace
#define ATTR_GZ "gz"
#define ATTR_NGZ "ngz"

#define DUMP_CFG(cfg, buf, sz)	\
	dump_cfg((cfg), (buf), (sz)); \
	fprintf(stderr, "CFG: %s\n", (buf));
#define MAX_METRIC_PREFIX_SZ	ZONENAME_MAX+64	// SOLMEX_FS_NAME_PREFIX "autofs{" ATTR_NGZ  "='',op=''}""

void
collect_fs(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now, void *cfg) {
	kstat_t *ksp;
	kstat_named_t *knp;
	char buf[32], *s;
	ks_info_idx_t idx;

	size_t psz = 0;
	int z, zones;
	uint_t e;
	vopstats_t vi;
	zinfo_t *zi;
	bool revalidate = false;

	static fs_mods_t mods = FS_MODS_NONE;	// set of fs regarding all zones
	static fs_mods_t *z_fs_mods = NULL;		// kstat instance related fs
	static char **znames = NULL;			// kstat instance related zonenames
	static int zlen = 0;					// capacity of z_fs_mods and znames
	static int last_zones = 0;				// the number of kstat instances from the previous run
	static kid_t last_kid = -1;				// the kstat id from the previous run
	static zinfo_t *last_cfg = NULL;		// cfg from previous run

	char metric_prefix[MAX_METRIC_PREFIX_SZ];

	PROM_DEBUG("collect_vopstats ...", "");
	if (cfg == NULL)
		return;

	bool free_sb = sb == NULL;
	if (free_sb) {
		sb = psb_new();
		if (sb == NULL) {
			PROM_WARN("collect_mib: %s", strerror(errno));
			return;
		}
	}
	if (last_kid != kc->kc_chain_id || last_cfg != cfg) {
		revalidate = true;
		last_kid = kc->kc_chain_id;
	}
	psz = psb_len(sb);
	check_zone_vars();
	last_cfg = (zinfo_t *) cfg;
	if (revalidate) {
		zi = last_cfg;
		mods = FS_MODS_NONE;
		while (zi) {
			if (zi->zid != ANY_ZID)
				zi->zid = getzoneidbyname(zi->zname);
			if (zi->zid != -1)
				mods |= zi->mods;
			zi = zi->next;
		}
		if (last_cfg->next)
			last_cfg->next = sortZinfos(last_cfg->next);
	}
	//DUMP_CFG(last_cfg, metric_prefix, MAX_METRIC_PREFIX_SZ);

	// update kstat and related instance for each fs
	for (idx=0; idx < KS_IDX_MAX; idx++) {
		if (GET_FS_MOD(mods, idx) == 0)
			continue;

		if ((zones = update_instance(kc, &kstat[idx])) == 0)
			continue;

		// cache zonenames and their related fs_mods if not already done
		if (revalidate || last_zones != zones) {
			if (zones > zlen) {
				// we only grow
				fs_mods_t *p1 = realloc(z_fs_mods, zones * sizeof(zinfo_t *));
				char **p2 = realloc(znames, zones * sizeof(char *));
				if (p1 == NULL || p2 == NULL) {
					PROM_WARN("Unable to allocate zone info tables: %s", strerror(errno));
					break;
				}
				z_fs_mods = p1;
				znames = p2;
				for (int i = last_zones; i < zones; i++) {
					z_fs_mods[i] = FS_MODS_NONE;
					znames[i] = NULL;
				}
				zlen = zones;
			}
			for (z = 0; z < zones; z++) {
				zi = last_cfg;
				free(znames[z]);
				znames[z] = NULL;
				z_fs_mods[z] = FS_MODS_NONE;
				while (zi) {
					if (zi->zid == kstat[idx].ksp[z]->ks_instance) {
						z_fs_mods[z] = zi->mods;
						znames[z] = strdup(zi->zname);
						break;
					} else if (zi->zid == ANY_ZID) {
						char zname[ZONENAME_MAX];
						z_fs_mods[z] = zi->mods;
						znames[z] = getzonenamebyid(kstat[idx]
							.ksp[z]->ks_instance, zname, ZONENAME_MAX) != -1
							? strdup(zname)
							: NULL;
						break;
					}
					zi = zi->next;
				}
			}
			last_zones = zones;
			revalidate = false;
		}

		// go through each instance (instance# == zoneid) of the vopstats_$fs
		// and collect the data if needed/available.
		for (z = 0; z < zones; z++) {
			if (znames[z] == NULL || (GET_FS_MOD(z_fs_mods[z], idx) == 0))
				continue;
			if ((ksp = ks_read(kc, kstat[idx].ksp[z], now, &vi)) == NULL)
				continue;
			if (ksp->ks_ndata == 0)
				continue;

			if (!compact) {
				psb_add_str(sb, "# ");
				psb_add_str(sb, ksp->ks_name + strlen(VOPSTATS_STR));
				psb_add_char(sb, '\n');
			}
			size_t tsz = psb_len(sb);
			psb_add_str(sb, SOLMEX_FS_NAME_PREFIX);
			psb_add_str(sb, ksp->ks_name + strlen(VOPSTATS_STR));
			psb_add_str(sb, "{" ATTR_NGZ "=\"");
			psb_add_str(sb, znames[z]);
			psb_add_str(sb, "\",op=\"");
			strcpy(metric_prefix, psb_str(sb) + tsz);
			knp = KSTAT_NAMED_PTR(ksp);
			/*
			Consistency checks:
			zoneadm list -iv
			setenv ZNAME ...
			setenv ZID ...
			setenv FSTYP ...
			solmex -cF -i x -m x -t x -T 'P:.*' -n version -z any:
			kstat :${ZID}:vopstats_${FSTYP} | tail +3 | \
			  gsed -re '/^[[:space:]]*([acs]|$)/ d' -e '/^[[:space:]]+n/ s/n//'\
			    -e '/^[[:space:]]+a.[^cd]/ d' | sort -k1
			curl -s localhost:9100/metrics | \
				egrep "_${FSTYP}\{ngz=.${ZNAME}.," | sort -t\" -k4
			*/
			for (e = 0; e < ksp->ks_ndata; e++, knp++) {
				s = knp->name;
				if (s[0] < 'd' || s[0] == 's') {
					// skip a{read,write}*, crtime and snaptime
					continue;
				}
				// all data are KSTAT_DATA_UINT64
				if (s[0] == 'n')
					s++;
				if (s[0] == 'a' && s[2] != 'c' && s[2] != 'd') {
					// skip na{cancel,fsync,read,write}
					continue;
				}
				if (e)
					psb_add_str(sb, metric_prefix);
				psb_add_str(sb, s);
				sprintf(buf, "\"} %ld\n", knp->value.ui64);
				psb_add_str(sb, buf);
			}
		}
	}
	if (free_sb) {
		if (psb_len(sb) != psz)
			fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("collect_fsstat done", "");
}
