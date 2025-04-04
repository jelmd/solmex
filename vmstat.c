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
#include <strings.h>
#include <errno.h>
#include <unistd.h>

#include <libprom/prom.h>

#include "ks_util.h"
#include "vmstat.h"

typedef enum ks_info_idx {
	KS_IDX_CPU_VM = 0,
	KS_IDX_MAX,			// last entry by contract
} ks_info_idx_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	{ "cpu", -1, "vm", -1, 0, NULL },
};
#pragma GCC diagnostic pop

/*
 * Ja hier sind wir etwas schreibfaul und machen das mal generisch, da man
 * hiervon nahezu alles gebrauchen kann und das mehr als 10 Werte/CPU sind:
kstat cpu:0:vm | tail +3 | awk '{ if ($1 ~ "crtime|snaptime|^$") next; \
	X = X "\"" $1 "\", "; Y = Y "VM_IDX_" toupper($1) ", "; \
	D = D "SOLMEX_VM_" toupper($1) "_D, " } \
	END { print "static const char *snames[] = {" X \
	"NULL\n};\ntypedef enum vm_idx {" Y \
	"VM_IDX_MAX} vm_idx_t;\nstatic const char *sdesc[] = {" D "NULL\n};" }'
*/
static const char *snames[] = {
	"anonfree", "anonpgin", "anonpgout", "as_fault", "cow_fault", "dfree",
	"execfree", "execpgin", "execpgout", "fsfree", "fspgin", "fspgout",
	"hat_fault", "kernel_asflt", "maj_fault", "pgfrec", "pgin", "pgout",
	"pgpgin", "pgpgout", "pgrec", "pgrrun", "pgswapin", "pgswapout",
	"prot_fault", "rev", "scan", "softlock", "swapin", "swapout", "zfod", NULL
};
static const char *sdesc[] = {
	SOLMEX_VM_ANONFREE_D,	SOLMEX_VM_ANONPGIN_D,		SOLMEX_VM_ANONPGOUT_D,
	SOLMEX_VM_AS_FAULT_D,	SOLMEX_VM_COW_FAULT_D,		SOLMEX_VM_DFREE_D,
	SOLMEX_VM_EXECFREE_D,	SOLMEX_VM_EXECPGIN_D,		SOLMEX_VM_EXECPGOUT_D,
	SOLMEX_VM_FSFREE_D,		SOLMEX_VM_FSPGIN_D,			SOLMEX_VM_FSPGOUT_D,
	SOLMEX_VM_HAT_FAULT_D,	SOLMEX_VM_KERNEL_ASFLT_D,	SOLMEX_VM_MAJ_FAULT_D,
	SOLMEX_VM_PGFREC_D,		SOLMEX_VM_PGIN_D,			SOLMEX_VM_PGOUT_D,
	SOLMEX_VM_PGPGIN_D,		SOLMEX_VM_PGPGOUT_D,		SOLMEX_VM_PGREC_D,
	SOLMEX_VM_PGRRUN_D,		SOLMEX_VM_PGSWAPIN_D,		SOLMEX_VM_PGSWAPOUT_D,
	SOLMEX_VM_PROT_FAULT_D,	SOLMEX_VM_REV_D,			SOLMEX_VM_SCAN_D,
	SOLMEX_VM_SOFTLOCK_D,	SOLMEX_VM_SWAPIN_D,			SOLMEX_VM_SWAPOUT_D,
	SOLMEX_VM_ZFOD_D,		NULL
};
typedef enum vm_idx
{
	VM_IDX_ANONFREE,	VM_IDX_ANONPGIN,		VM_IDX_ANONPGOUT,
	VM_IDX_AS_FAULT,	VM_IDX_COW_FAULT,		VM_IDX_DFREE,
	VM_IDX_EXECFREE,	VM_IDX_EXECPGIN,		VM_IDX_EXECPGOUT,
	VM_IDX_FSFREE,		VM_IDX_FSPGIN,			VM_IDX_FSPGOUT,
	VM_IDX_HAT_FAULT,	VM_IDX_KERNEL_ASFLT,	VM_IDX_MAJ_FAULT,
	VM_IDX_PGFREC,		VM_IDX_PGIN,			VM_IDX_PGOUT,
	VM_IDX_PGPGIN,		VM_IDX_PGPGOUT,			VM_IDX_PGREC,
	VM_IDX_PGRRUN,		VM_IDX_PGSWAPIN,		VM_IDX_PGSWAPOUT,
	VM_IDX_PROT_FAULT,	VM_IDX_REV,				VM_IDX_SCAN,
	VM_IDX_SOFTLOCK,	VM_IDX_SWAPIN,			VM_IDX_SWAPOUT,
	VM_IDX_ZFOD,		VM_IDX_MAX
} vm_idx_t;

static vm_idx_t nstats[] = {
	// unix::vminfo  swap_avail	(swap -s)
	VM_IDX_SWAPIN,	VM_IDX_SWAPOUT,
	VM_IDX_PGREC,	VM_IDX_PGFREC,	VM_IDX_HAT_FAULT,	VM_IDX_AS_FAULT,
	VM_IDX_PGPGIN,	VM_IDX_PGPGOUT,	VM_IDX_DFREE, 		VM_IDX_SCAN,
	// unix::system_misc  deficit	/* estimate of needs of new swapped in procs */
};
static uint32_t nstats_sz = ARRAY_SIZE(nstats);

static vm_idx_t xstats[] = {
	VM_IDX_EXECPGIN, VM_IDX_EXECPGOUT, VM_IDX_EXECPGOUT,
	VM_IDX_ANONPGIN, VM_IDX_ANONPGOUT, VM_IDX_ANONFREE,
	VM_IDX_FSPGIN, VM_IDX_FSPGOUT, VM_IDX_FSFREE
};
static uint32_t xstats_sz = ARRAY_SIZE(xstats);

static vm_idx_t astats[VM_IDX_MAX];
static uint32_t astats_sz = VM_IDX_MAX;

static uint16_t strand_count_last = 0;

void
collect_vmstat(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now,
	bool mp, vm_stat_quantity_t stype)
{
	static uint64_t *vals = NULL;
	static int *seen = NULL;
	vm_idx_t *what;

	kstat_t *ksp;
	kstat_named_t *knp;
	char buf[64];
	int i, k;
	uint64_t idx,sidx;
	uint32_t what_sz, l;
	vm_stat_quantity_t tmp_type;

	bool free_sb = sb == NULL;

	PROM_DEBUG("collect_vmstats ...", "");
	if (stype == VMSTAT_NONE)
		return;

	uint16_t n = update_instance(kc, &kstat[KS_IDX_CPU_VM]);
	if (n == 0)
		return;

	if (n > system_cpu_max) {
		PROM_WARN("Possibly tinkered system (%d strands) - ignored", n);
		return;
	}

	if (mp && n == 1)
		mp = false;
	if (n > strand_count_last) {
		uint64_t *v = realloc(vals, (n + 1) * sizeof(uint64_t) * VM_IDX_MAX);
		int *s = realloc(seen, (n + 1) * sizeof(int));
		if (v == NULL || s == NULL) {
			PROM_WARN("Memory problem in vmstats: %s", strerror(errno));
			free(v);
			free(s);
			return;
		}
		vals = v;
		seen = s;
		memset(vals + strand_count_last * VM_IDX_MAX, 0,
			(n + 1 - strand_count_last) * sizeof(uint64_t) * VM_IDX_MAX);
	} else {
		memset(vals + n * VM_IDX_MAX, 0, VM_IDX_MAX * sizeof(uint64_t)); // reset sum
		memset(seen, 0, n * sizeof(int));
	}
	strand_count_last = n;
	seen[n] = n;
	// too lazy to init it manually ;-)
	if (stype == VMSTAT_ALL && astats[1] == 0) {
		for (k = 0; k < VM_IDX_MAX; k++)
			astats[k] = k;
	}

	if (free_sb)
		sb = psb_new();

	// get stats for each strand
	sidx = n * VM_IDX_MAX;
	for (i = idx = 0; i < n; i++, idx += VM_IDX_MAX) {
		if ((ksp = ks_read(kc, kstat[KS_IDX_CPU_VM].ksp[i], now, NULL)) == NULL)
			continue;
		seen[i] = ksp->ks_instance + 1;	// instance start with 0 ;-)
		tmp_type = stype;
		if (stype == VMSTAT_ALL) {
			what = astats;
			what_sz = astats_sz;
		} else {
			what = nstats;
			what_sz = nstats_sz;
		}
getx:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
		for (l = 0; l < what_sz; l++) {
			k = what[l];
			if ((knp = kstat_data_lookup(ksp, snames[k])) != NULL) {
				vals[sidx + k] += knp->value.ui64;
				if (mp)
					vals[idx + k] = knp->value.ui64;
			}
		}
#pragma GCC diagnostic pop
		if (tmp_type == VMSTAT_EXTENDED) {
			what = xstats;
			what_sz = xstats_sz;
			tmp_type = VMSTAT_NONE;
			goto getx;
		}
	}

	//print stats for each strand or just the summary
	tmp_type = stype;
	if (stype == VMSTAT_ALL) {
		what = astats;
		what_sz = astats_sz;
	} else {
		what = nstats;
		what_sz = nstats_sz;
	}
valx:
	for (l = 0; l < what_sz; l++) {
		k = what[l];
		if (!compact)
			addPromInfo4(SOLMEX_VM_NAME_PREFIX, snames[k], "counter", sdesc[k]);
		idx = 0;
		for (i = mp ? 0 : n; i <= n; i++, idx += VM_IDX_MAX) {
			if (!seen[i])
				continue;
			psb_add_str(sb, SOLMEX_VM_NAME_PREFIX);
			psb_add_str(sb, snames[k]);
			if (i == n) {
				sprintf(buf, "{cpu=\"sum\"} %ld\n", vals[sidx + k]);
				psb_add_str(sb, buf);
			} else if (mp) {
				sprintf(buf, "{cpu=\"%d\"} %ld\n", seen[i] - 1, vals[idx + k]);
				psb_add_str(sb, buf);
			}
		}
	}
	if (tmp_type == VMSTAT_EXTENDED) {
		what = xstats;
		what_sz = xstats_sz;
		tmp_type = VMSTAT_NONE;
		goto valx;
	}

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}

	PROM_DEBUG("collect_vmstats done", "");
}
