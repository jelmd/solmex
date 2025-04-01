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

#ifndef SOLMEX_COMMON_H
#define SOLMEX_COMMON_H

#define SOLMEX_VERSION "1.0.0"
#define SOLMEX_AUTHOR "Jens Elkner (jel+solmex@cs.uni-magdeburg.de)"

#include <stdbool.h>
#include <regex.h>

#include <libprom/prom_string_builder.h>
#include <libprom/prom_log.h>

#ifdef __cplusplus
extern "C" {
#endif

/** max. number of CPUs in a system. 8 is probably in most cases sufficient. */
#define MAX_CPUS 64

/** The number of CPU strands alias hyperthreads on the system. */
extern uint8_t system_cpu_count;

#define MBUF_SZ 256
#define ARRAY_SIZE(array) (sizeof (array) / sizeof (array[0]))

typedef struct node_cfg {
	bool no_dmi;
	bool no_kstats;
	bool no_load;
	bool no_cpu_state;
	bool no_cpu_speed;
	bool no_cpu_speed_max;
	bool no_sys_mem;
	regex_t *exc_metrics;
	regex_t *exc_sensors;
	regex_t *inc_metrics;
	regex_t *inc_sensors;
} node_cfg_t;

#define addPromInfo(metric) {\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# HELP " metric ## _N " " metric ## _D );\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# TYPE " metric ## _N " " metric ## _T);\
	psb_add_char(sb, '\n');\
}

#define SOLMEXM_VERS_D "Software version information."
#define SOLMEXM_VERS_T "gauge"
#define SOLMEXM_VERS_N "solmex_version"

// most of names and types are choosen to be node-exporter compatible, even so
// there are many misnomers and disagreements wrt. type ;-)
#define SOLMEXM_DMI_D "A constant metric with label entries deduced from the DMI (see smbios(1M) type 0 .. 5). Always 1."
#define SOLMEXM_DMI_T "gauge"
#define SOLMEXM_DMI_N "solmex_node_dmi_info"

#define SOLMEXM_BOOTTIME_D "OS boot time, in seconds since the Epoch (00:00:00 UTC, January 1, 1970). Constant for the lifetime of the app."
#define SOLMEXM_BOOTTIME_T "gauge"
#define SOLMEXM_BOOTTIME_N "solmex_node_boot_time_seconds"

#define SOLMEXM_CPUINFO_D "Information about system CPUs. Constant for the lifetime of the app. Always 1."
#define SOLMEXM_CPUINFO_T "gauge"
#define SOLMEXM_CPUINFO_N "solmex_node_cpu_info"


#define SOLMEXM_LOAD1_D "Number of processes in the system run queue as average over the last minute."
#define SOLMEXM_LOAD1_T "gauge"
#define SOLMEXM_LOAD1_N "solmex_node_load1"

#define SOLMEXM_LOAD5_D "Number of processes in the system run queue as average over the last 5 minutes."
#define SOLMEXM_LOAD5_T "gauge"
#define SOLMEXM_LOAD5_N "solmex_node_load5"

#define SOLMEXM_LOAD15_D "Number of processes in the system run queue as average over the last 15 minutes."
#define SOLMEXM_LOAD15_T "gauge"
#define SOLMEXM_LOAD15_N "solmex_node_load15"


#define SOLMEXM_CPUSTATE_D "Total number of CPU strands alias hyperthreads in the system."
#define SOLMEXM_CPUSTATE_T "gauge"
#define SOLMEXM_CPUSTATE_N "solmex_node_cpus_total"

#define SOLMEXM_CPUSPEED_D "Current CPU strand alias thread frequency in hertz."
#define SOLMEXM_CPUSPEED_T "gauge"
#define SOLMEXM_CPUSPEED_N "solmex_node_cpu_frequency_hertz"

#define SOLMEXM_CPUSPEEDMAX_D "Max. CPU strand alias thread frequency in hertz."
#define SOLMEXM_CPUSPEEDMAX_T "gauge"
#define SOLMEXM_CPUSPEEDMAX_N "solmex_node_cpu_frequency_max_hertz"


// Lower memory limit: if crossed page out and swap ops get highest priority,
// page scanning becomes more intensive. Default: desfree/2
#define SOLMEXM_MINFREE_D "Minimal amount of free memory which is tolerable. If below run page scanner every time a new page is created. Default: desfree/2"
#define SOLMEXM_MINFREE_T "gauge"
#define SOLMEXM_MINFREE_N "solmex_node_mem_minfree"

// The desired amount of memory to be free at all times on the system. If ~30s
// below this limit, system starts swapping. Default: lotsfree/2
#define SOLMEXM_DESFREE_D "Amount of memory desired free. If below for ~ 30s run page scanner 100 instead off 4 times/s. Default: lotsfree/2"
#define SOLMEXM_DESFREE_T "gauge"
#define SOLMEXM_DESFREE_N "solmex_node_mem_desfree"

#define SOLMEXM_LOTSFREE_D "Threshold when paging scanner wakes up to reclaim memory. Default: physmem/64"
#define SOLMEXM_LOTSFREE_T "gauge"
#define SOLMEXM_LOTSFREE_N "solmex_node_mem_lotsfree"

#define SOLMEXM_NSCAN_D "Memory in bytes scanned during the last second."
#define SOLMEXM_NSCAN_T "gauge"
#define SOLMEXM_NSCAN_N "solmex_node_mem_nscan"

// The maximum number of pages per second that the system looks at when memory
// pressure is highest.
#define SOLMEXM_FASTSCAN_D "Memory in bytes scanned per second when free memory falls below minfree."
#define SOLMEXM_FASTSCAN_T "gauge"
#define SOLMEXM_FASTSCAN_N "solmex_node_mem_fastscan"

#define SOLMEXM_DESSCAN_D "Desired memory in bytes scanned per second. Max. <= fastscan/4."
#define SOLMEXM_DESSCAN_T "gauge"
#define SOLMEXM_DESSCAN_N "solmex_node_mem_desscan"

// The minimum number of pages per second that the system looks at when
// attempting to reclaim memory.
#define SOLMEXM_SLOWSCAN_D "Memory in bytes scanned per second when free memory falls below lotsfree. Max. <= fastscan/2."
#define SOLMEXM_SLOWSCAN_T "gauge"
#define SOLMEXM_SLOWSCAN_N "solmex_node_mem_slowscan"

#define SOLMEXM_PHYSMEM_D "Total physical RAM of the system in bytes."
#define SOLMEXM_PHYSMEM_T "gauge"
#define SOLMEXM_PHYSMEM_N "solmex_node_mem_phys"

#define SOLMEXM_AVAILRMEM_D "Available resident (pageable, unreserved) physical memory in bytes."
#define SOLMEXM_AVAILRMEM_T "gauge"
#define SOLMEXM_AVAILRMEM_N "solmex_node_mem_availr"

#define SOLMEXM_LOCKEDMEM_D "Physical memory in bytes locked because of user specified locking through mlock or plock. Should be ~ (phys - availr)"
#define SOLMEXM_LOCKEDMEM_T "gauge"
#define SOLMEXM_LOCKEDMEM_N "solmex_node_mem_locked"

#define SOLMEXM_FREEMEM_D "Currently re-usable memory in bytes. It includes memory paged out and used for cache, but page scanner prefers unused pages."
#define SOLMEXM_FREEMEM_T "gauge"
#define SOLMEXM_FREEMEM_N "solmex_node_mem_free"

#define SOLMEXM_PPKERNEL_D "Total physical memory in bytes used by the kernel since the startup."
#define SOLMEXM_PPKERNEL_T "gauge"
#define SOLMEXM_PPKERNEL_N "solmex_node_mem_kernel"

#define SOLMEXM_PAGESZ_D "Size in bytes of system memory pages."
#define SOLMEXM_PAGESZ_T "gauge"
#define SOLMEXM_PAGESZ_N "solmex_node_mem_pagesz"

#define SOLMEXM_NALLOC_D "Total number of memory allocation calls. Includes failed requests, too."
#define SOLMEXM_NALLOC_T "counter"
#define SOLMEXM_NALLOC_N "solmex_node_mem_nalloc_calls"

#define SOLMEXM_NALLOCSZ_D "Total memory in bytes requested by allocation calls. Includes size of failed requests, too."
#define SOLMEXM_NALLOCSZ_T "counter"
#define SOLMEXM_NALLOCSZ_N "solmex_node_mem_nalloc_sz"

#define SOLMEXM_NFREE_D "Total number of call to free allocated memory."
#define SOLMEXM_NFREE_T "counter"
#define SOLMEXM_NFREE_N "solmex_node_mem_free_calls"

#define SOLMEXM_NFREESZ_D "Total allocated memory freed in bytes."
#define SOLMEXM_NFREESZ_T "counter"
#define SOLMEXM_NFREESZ_N "solmex_node_mem_free_sz"

/*
#define SOLMEXM_XXX_D "short description."
#define SOLMEXM_XXX_T "gauge"
#define SOLMEXM_XXX_N "solmex_yyy"
 */

#ifdef __cplusplus
}
#endif

#endif // SOLMEX_COMMON_H
