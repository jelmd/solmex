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

/*
#define SOLMEXM_XXX_D "short description."
#define SOLMEXM_XXX_T "gauge"
#define SOLMEXM_XXX_N "solmex_yyy"

 */

#ifdef __cplusplus
}
#endif

#endif // SOLMEX_COMMON_H
