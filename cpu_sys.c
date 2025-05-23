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
#include "cpu_sys.h"

typedef enum ks_info_idx {
	KS_IDX_CPU_VM = 0,
	KS_IDX_MAX,			// last entry by contract
} ks_info_idx_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
static ks_info_t kstat[KS_IDX_MAX] = {
	{ "cpu", -1, "sys", -1, 0, NULL },
};
#pragma GCC diagnostic pop

/** Ja, auch hier wieder etwas schreibfaul, also auch generisch ;-) */
static const char *knames[] = {
	"cpu_ticks_idle",	"cpu_ticks_kernel",	"cpu_ticks_user",
	"cpu_ticks_wait",	"intr",				"inv_swtch",
	"namei",			"nthreads",			"phread",
	"phwrite",			"pswitch",			"syscall",
	"sysexec",			"sysfork",			"sysread",
	"sysvfork",			"syswrite",			"trap",			"xcalls",
	// to be able to simulate mpstat:
	"intrthread",		"cpumigrate",		"mutex_adenters",
	"rw_rdfails",		"rw_wrfails",		NULL
};

typedef enum sys_idx {
	SYS_IDX_CPU_TICKS_IDLE,	SYS_IDX_CPU_TICKS_KERNEL,	SYS_IDX_CPU_TICKS_USER,
	SYS_IDX_CPU_TICKS_WAIT,	SYS_IDX_INTR,				SYS_IDX_INV_SWTCH,
	SYS_IDX_NAMEI,			SYS_IDX_NTHREADS,			SYS_IDX_PHREAD,
	SYS_IDX_PHWRITE,		SYS_IDX_PSWITCH,			SYS_IDX_SYSCALL,
	SYS_IDX_SYSEXEC,		SYS_IDX_SYSFORK,			SYS_IDX_SYSREAD,
	SYS_IDX_SYSVFORK,		SYS_IDX_SYSWRITE,			SYS_IDX_TRAP,
	SYS_IDX_XCALLS,			SYS_IDX_IRQTHREAD,			SYS_IDX_MIGRATE,
	SYS_IDX_MUTEXIN,		SYS_IDX_RDFAILS,			SYS_IDX_WRFAILS,
	SYS_IDX_MAX
} sys_idx_t;

/* solmex metric names */
static const char *snames[] = {
#define STRINGIFY(x) #x
#define _S(x) STRINGIFY(x)
	_S(SOLMEX_SYS_CPU_TICKS_IDLE_N),	_S(SOLMEX_SYS_CPU_TICKS_KERNEL_N),	_S(SOLMEX_SYS_CPU_TICKS_USER_N),
	_S(SOLMEX_SYS_CPU_TICKS_WAIT_N),	_S(SOLMEX_SYS_INTR_N),				_S(SOLMEX_SYS_INV_SWTCH_N),
	_S(SOLMEX_SYS_NAMEI_N),				_S(SOLMEX_SYS_NTHREADS_N),			_S(SOLMEX_SYS_PHREAD_N),
	_S(SOLMEX_SYS_PHWRITE_N),			_S(SOLMEX_SYS_PSWITCH_N),			_S(SOLMEX_SYS_SYSCALL_N),
	_S(SOLMEX_SYS_SYSEXEC_N),			_S(SOLMEX_SYS_SYSFORK_N),			_S(SOLMEX_SYS_SYSREAD_N),
	_S(SOLMEX_SYS_SYSVFORK_N),			_S(SOLMEX_SYS_SYSWRITE_N),			_S(SOLMEX_SYS_TRAP_N),
	_S(SOLMEX_SYS_XCALLS_N),
	// to be able to simulate mpstat:
	_S(SOLMEX_SYS_IRQTHREAD_N),			_S(SOLMEX_SYS_MIGRATE_N),
	_S(SOLMEX_SYS_MUTEXIN_N),			_S(SOLMEX_SYS_RDFAILS_N),			_S(SOLMEX_SYS_WRFAILS_N),
	NULL
};

static const char *sdesc[] = {
	SOLMEX_SYS_CPU_TICKS_IDLE_D,	SOLMEX_SYS_CPU_TICKS_KERNEL_D,	SOLMEX_SYS_CPU_TICKS_USER_D,
	SOLMEX_SYS_CPU_TICKS_WAIT_D,	SOLMEX_SYS_INTR_D,				SOLMEX_SYS_INV_SWTCH_D,
	SOLMEX_SYS_NAMEI_D,				SOLMEX_SYS_NTHREADS_D,			SOLMEX_SYS_PHREAD_D,
	SOLMEX_SYS_PHWRITE_D,			SOLMEX_SYS_PSWITCH_D,			SOLMEX_SYS_SYSCALL_D,
	SOLMEX_SYS_SYSEXEC_D,			SOLMEX_SYS_SYSFORK_D,			SOLMEX_SYS_SYSREAD_D,
	SOLMEX_SYS_SYSVFORK_D,			SOLMEX_SYS_SYSWRITE_D,			SOLMEX_SYS_TRAP_D,
	SOLMEX_SYS_XCALLS_D,			
	// to be able to simulate mpstat:
	SOLMEX_SYS_IRQTHREAD_D,			SOLMEX_SYS_MIGRATE_D,
	SOLMEX_SYS_MUTEXIN_D,			SOLMEX_SYS_RDFAILS_D,	SOLMEX_SYS_WRFAILS_D,
	NULL
};

static sys_idx_t nstats[] = {
	SYS_IDX_CPU_TICKS_IDLE,	SYS_IDX_CPU_TICKS_KERNEL,	SYS_IDX_CPU_TICKS_USER,
	SYS_IDX_CPU_TICKS_WAIT,	SYS_IDX_INTR,				SYS_IDX_PSWITCH,
	SYS_IDX_SYSCALL,		SYS_IDX_SYSEXEC,			SYS_IDX_SYSFORK,
	SYS_IDX_SYSVFORK,		SYS_IDX_TRAP
};
static uint32_t nstats_sz = ARRAY_SIZE(nstats);

static sys_idx_t xstats[] = {
	SYS_IDX_INV_SWTCH,	SYS_IDX_NAMEI,		SYS_IDX_NTHREADS,
	SYS_IDX_PHREAD,		SYS_IDX_PHWRITE,	SYS_IDX_SYSREAD,
	SYS_IDX_SYSWRITE,	SYS_IDX_XCALLS,
	// to be able to simulate mpstat:
	SYS_IDX_IRQTHREAD,	SYS_IDX_MIGRATE,
	SYS_IDX_MUTEXIN,	SYS_IDX_RDFAILS,	SYS_IDX_WRFAILS,
};
static uint32_t xstats_sz = ARRAY_SIZE(xstats);


void
collect_cpusys(psb_t *sb, bool compact, kstat_ctl_t *kc, hrtime_t now,
	bool mp, cpu_sys_quantity_t stype)
{
	static uint16_t strand_count_last = 0;
	static uint64_t *vals = NULL;
	static int *seen = NULL;
	sys_idx_t *what;

	kstat_t *ksp;
	kstat_named_t *knp;
	char buf[64];
	int i, k;
	uint64_t idx, sidx;
	uint32_t what_sz, l;
	cpu_sys_quantity_t tmp_type;

	bool free_sb = sb == NULL;

	PROM_DEBUG("collect_cpusys ...", "");
	if (stype == CPUSYS_NONE)
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
		uint64_t *v = realloc(vals, (n + 1) * sizeof(uint64_t) * SYS_IDX_MAX);
		int *s = realloc(seen, (n + 1) * sizeof(int));
		if (v == NULL || s == NULL) {
			PROM_WARN("Memory problem in cpu_sys: %s", strerror(errno));
			free(v);
			free(s);
			return;
		}
		vals = v;
		seen = s;
		memset(vals + strand_count_last * SYS_IDX_MAX, 0,
			(n + 1 - strand_count_last) * sizeof(uint64_t) * SYS_IDX_MAX);
	} else {
		memset(vals + n * SYS_IDX_MAX, 0, SYS_IDX_MAX * sizeof(uint64_t)); // reset sum
	}
	strand_count_last = n;
	memset(seen, 0, n * sizeof(int));
	seen[n] = n;	// last row is alway valid - the sum over all CPUs

	if (free_sb)
		sb = psb_new();

	// get stats for each strand
	sidx = n * SYS_IDX_MAX;
	for (i = idx = 0; i < n; i++, idx += SYS_IDX_MAX) {
		if ((ksp = ks_read(kc, kstat[KS_IDX_CPU_VM].ksp[i], now, NULL)) == NULL)
			continue;
		seen[i] = ksp->ks_instance + 1;	// instance start with 0 ;-)
		tmp_type = stype;
		what = nstats;
		what_sz = nstats_sz;
getx:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
		for (l = 0; l < what_sz; l++) {
			k = what[l];
			if ((knp = kstat_data_lookup(ksp, knames[k])) != NULL) {
				vals[sidx + k] += knp->value.ui64;
				if (mp)
					vals[idx + k] = knp->value.ui64;
			}
		}
#pragma GCC diagnostic pop
		if (tmp_type == CPUSYS_EXTENDED) {
			what = xstats;
			what_sz = xstats_sz;
			tmp_type = CPUSYS_NONE;
			goto getx;
		}
	}

	//print stats for each strand or just the summary
	tmp_type = stype;
	what = nstats;
	what_sz = nstats_sz;
valx:
	for (l = 0; l < what_sz; l++) {
		k = what[l];
		if (!compact)
			addPromInfo4("", snames[k], "counter", sdesc[k]);
		idx = 0;
		for (i = mp ? 0 : n; i <= n; i++, idx += SYS_IDX_MAX) {
			if (!seen[i])
				continue;
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
	if (tmp_type == CPUSYS_EXTENDED) {
		what = xstats;
		what_sz = xstats_sz;
		tmp_type = CPUSYS_NONE;
		goto valx;
	}

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}

	PROM_DEBUG("collect_cpusys done", "");
}
