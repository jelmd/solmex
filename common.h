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

/** The number of CPU strands alias hyperthreads found on the system when the
 * first collector (cpu_info) got called. */
extern uint16_t system_cpu_count;
/** The max. CPU strand alias hyperthread ID supported on this system */
extern uint16_t system_cpu_max;
/** The systems default memory pape size */
extern uint64_t page_sz;
/** The systems default memory pape size as shift operand */
extern uint8_t page_shift;
/** Number of ticks per second */
extern uint64_t tps;

#define MBUF_SZ 256
#define ARRAY_SIZE(array) (sizeof (array) / sizeof (array[0]))

#define addPromInfo(metric) {\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# HELP " metric ## _N " " metric ## _D );\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# TYPE " metric ## _N " " metric ## _T);\
	psb_add_char(sb, '\n');\
}

#define addPromInfo4(prefix, name, type, desc) {\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# HELP ");\
	psb_add_str(sb, prefix);\
	psb_add_str(sb, name);\
	psb_add_char(sb, ' ');\
	psb_add_str(sb, desc);\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# TYPE ");\
	psb_add_str(sb, prefix);\
	psb_add_str(sb, name);\
	psb_add_char(sb, ' ');\
	psb_add_str(sb, type);\
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

#define SOLMEXM_DEFICIT_D "Estimate of needs of new swapped in procs ('de' in vmstat)"
#define SOLMEXM_DEFICIT_T "gauge"
#define SOLMEXM_DEFICIT_N "solmex_node_load_memdeficit_bytes"


#define SOLMEXM_UNIT_TICKS_D "Number of ticks per second"
#define SOLMEXM_UNIT_TICKS_T "gauge"
#define SOLMEXM_UNIT_TICKS_N "solmex_node_unit_second_ticks"

#define SOLMEXM_UNIT_PAGE_D "Size of system memory pages"
#define SOLMEXM_UNIT_PAGE_T "gauge"
#define SOLMEXM_UNIT_PAGE_N "solmex_node_unit_page_bytes"


#define SOLMEXM_PROCQ_RUN_D "Average number of processes per second put into the run queue since last query ('r' in vmstat)"
#define SOLMEXM_PROCQ_RUN_T "gauge"
#define SOLMEXM_PROCQ_RUN_N "solmex_node_procq_run_pps"

#define SOLMEXM_PROCQ_WAIT_D "Average number of processes per second blocked waiting for I/O since last query ('b' in vmstat)"
#define SOLMEXM_PROCQ_WAIT_T "gauge"
#define SOLMEXM_PROCQ_WAIT_N "solmex_node_procq_wait_pps"

#define SOLMEXM_PROCQ_SWAP_D "Average number of idle processes per second that have been swapped since last query ('w' in vmsat)"
#define SOLMEXM_PROCQ_SWAP_T "gauge"
#define SOLMEXM_PROCQ_SWAP_N "solmex_node_procq_swap_pps"


#define SOLMEXM_SWAP_ALLOC_D "Allocated swap"
#define SOLMEXM_SWAP_ALLOC_T "gauge"
#define SOLMEXM_SWAP_ALLOC_N "solmex_node_swap_allocated_bytes"

#define SOLMEXM_SWAP_AVAIL_D "Unreserved swap ('swap' in vmstat and 'available' in 'swap -s')"
#define SOLMEXM_SWAP_AVAIL_T "gauge"
#define SOLMEXM_SWAP_AVAIL_N "solmex_node_swap_available_bytes"

#define SOLMEXM_SWAP_FREE_D "Unallocated swap"
#define SOLMEXM_SWAP_FREE_T "gauge"
#define SOLMEXM_SWAP_FREE_N "solmex_node_swap_free_bytes"

#define SOLMEXM_SWAP_RESV_D "The total amount of swap space not currently allocated, but claimed by memory mappings for possible future use ('used' in 'swap -s')"
#define SOLMEXM_SWAP_RESV_T "gauge"
#define SOLMEXM_SWAP_RESV_N "solmex_node_swap_resv_bytes"


#define SOLMEXM_CPUSTATE_D "Total number of CPU strands alias hyperthreads in the system."
#define SOLMEXM_CPUSTATE_T "gauge"
#define SOLMEXM_CPUSTATE_N "solmex_node_cpus_total"

#define SOLMEXM_CPUSPEED_D "Current CPU strand alias thread frequency"
#define SOLMEXM_CPUSPEED_T "gauge"
#define SOLMEXM_CPUSPEED_N "solmex_node_cpu_frequency_hertz"

#define SOLMEXM_CPUSPEEDMAX_D "Max. CPU strand alias thread frequency"
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

#define SOLMEXM_NSCAN_D "Memory in bytes scanned during the last second"
#define SOLMEXM_NSCAN_T "gauge"
#define SOLMEXM_NSCAN_N "solmex_node_mem_nscan"

// The maximum number of pages per second that the system looks at when memory
// pressure is highest.
#define SOLMEXM_FASTSCAN_D "Memory in bytes scanned per second when free memory falls below minfree. Min. 64 MiB or physmem/2"
#define SOLMEXM_FASTSCAN_T "gauge"
#define SOLMEXM_FASTSCAN_N "solmex_node_mem_fastscan"

#define SOLMEXM_DESSCAN_D "Desired memory in bytes scanned per second. Max. <= fastscan/4."
#define SOLMEXM_DESSCAN_T "gauge"
#define SOLMEXM_DESSCAN_N "solmex_node_mem_desscan"

// The minimum number of pages per second that the system looks at when
// attempting to reclaim memory.
#define SOLMEXM_SLOWSCAN_D "Memory in bytes scanned per second when free memory falls below lotsfree. Max. <= fastscan/2"
#define SOLMEXM_SLOWSCAN_T "gauge"
#define SOLMEXM_SLOWSCAN_N "solmex_node_mem_slowscan"

#define SOLMEXM_PHYSMEM_D "Total physical RAM of the system"
#define SOLMEXM_PHYSMEM_T "gauge"
#define SOLMEXM_PHYSMEM_N "solmex_node_mem_phys_bytes"

#define SOLMEXM_AVAILRMEM_D "Available resident (pageable, unreserved) physical memory"
#define SOLMEXM_AVAILRMEM_T "gauge"
#define SOLMEXM_AVAILRMEM_N "solmex_node_mem_availr_bytes"

#define SOLMEXM_LOCKEDMEM_D "Physical memory locked because of user specified locking through mlock or plock. Should be ~ (phys - availr)"
#define SOLMEXM_LOCKEDMEM_T "gauge"
#define SOLMEXM_LOCKEDMEM_N "solmex_node_mem_locked_bytes"

#define SOLMEXM_FREEMEM_D "Currently re-usable memory. It includes memory paged out and used for cache, but page scanner prefers unused pages ('free' in vmstat)"
#define SOLMEXM_FREEMEM_T "gauge"
#define SOLMEXM_FREEMEM_N "solmex_node_mem_free_bytes"

#define SOLMEXM_PPKERNEL_D "Total physical memory used by the kernel since the startup"
#define SOLMEXM_PPKERNEL_T "gauge"
#define SOLMEXM_PPKERNEL_N "solmex_node_mem_kernel_bytes"

#define SOLMEXM_NALLOC_D "Total number of memory allocations. Includes failed requests, too"
#define SOLMEXM_NALLOC_T "counter"
#define SOLMEXM_NALLOC_N "solmex_node_mem_nalloc_calls"

#define SOLMEXM_NALLOCSZ_D "Total memory requested by allocation calls. Includes size of failed requests, too"
#define SOLMEXM_NALLOCSZ_T "counter"
#define SOLMEXM_NALLOCSZ_N "solmex_node_mem_nalloc_bytes"

#define SOLMEXM_NFREE_D "Total calls to free allocated memory"
#define SOLMEXM_NFREE_T "counter"
#define SOLMEXM_NFREE_N "solmex_node_mem_nfree_calls"

#define SOLMEXM_NFREESZ_D "Total allocated memory freed"
#define SOLMEXM_NFREESZ_T "counter"
#define SOLMEXM_NFREESZ_N "solmex_node_mem_nfree_bytes"


#define SOLMEX_VM_NAME_PREFIX "solmex_node_vm_"
/*
kstat cpu:0:vm | tail +3 | awk -v P='#define SOLMEX_VM_' -v NP='solmex_node_vm_'\
	'{  ($1 ~ "crtime|snaptime|^$") next; S=P toupper($1) "_"; \
	print S "D \"\""; print S "T \"counter\"" ; \
	print S "N " NP $1 "\n"; }'
*/
#define SOLMEX_VM_ANONFREE_D "Anonymous page-frees ('apf' in 'vmstat -p')"
#define SOLMEX_VM_ANONFREE_T "counter"
#define SOLMEX_VM_ANONFREE_N solmex_node_vm_anonfree

#define SOLMEX_VM_ANONPGIN_D "Anonymous page-ins ('api' in 'vmstat -p')"
#define SOLMEX_VM_ANONPGIN_T "counter"
#define SOLMEX_VM_ANONPGIN_N solmex_node_vm_anonpgin

#define SOLMEX_VM_ANONPGOUT_D "Anonymous page-outs ('apo' in 'vmstat -p')"
#define SOLMEX_VM_ANONPGOUT_T "counter"
#define SOLMEX_VM_ANONPGOUT_N solmex_node_vm_anonpgout

#define SOLMEX_VM_AS_FAULT_D "Minor (as) faults (summand 2/2 of 'mf' in vmstat and 'minf' in mpstat)"
#define SOLMEX_VM_AS_FAULT_T "counter"
#define SOLMEX_VM_AS_FAULT_N solmex_node_vm_as_fault

#define SOLMEX_VM_COW_FAULT_D "Copy-on-write faults"
#define SOLMEX_VM_COW_FAULT_T "counter"
#define SOLMEX_VM_COW_FAULT_N solmex_node_vm_cow_fault

#define SOLMEX_VM_DFREE_D "Pages freed by daemon or auto ('fr' in vmstat)"
#define SOLMEX_VM_DFREE_T "counter"
#define SOLMEX_VM_DFREE_N solmex_node_vm_dfree

#define SOLMEX_VM_EXECFREE_D "Executable page-frees ('epf' in 'vmstat -p')"
#define SOLMEX_VM_EXECFREE_T "counter"
#define SOLMEX_VM_EXECFREE_N solmex_node_vm_execfree

#define SOLMEX_VM_EXECPGIN_D "Executable page-ins ('epi' in 'vmstat -p')"
#define SOLMEX_VM_EXECPGIN_T "counter"
#define SOLMEX_VM_EXECPGIN_N solmex_node_vm_execpgin

#define SOLMEX_VM_EXECPGOUT_D "Executable page-outs ('epo' in 'vmstat -p')"
#define SOLMEX_VM_EXECPGOUT_T "counter"
#define SOLMEX_VM_EXECPGOUT_N solmex_node_vm_execpgout

#define SOLMEX_VM_FSFREE_D "File system page-frees ('fpf' in 'vmstat -p')"
#define SOLMEX_VM_FSFREE_T "counter"
#define SOLMEX_VM_FSFREE_N solmex_node_vm_fsfree

#define SOLMEX_VM_FSPGIN_D "File system page-ins ('fpi' in 'vmstat -p')"
#define SOLMEX_VM_FSPGIN_T "counter"
#define SOLMEX_VM_FSPGIN_N solmex_node_vm_fspgin

#define SOLMEX_VM_FSPGOUT_D "File system page-outs ('fpo' in 'vmstat -p')"
#define SOLMEX_VM_FSPGOUT_T "counter"
#define SOLMEX_VM_FSPGOUT_N solmex_node_vm_fspgout

#define SOLMEX_VM_HAT_FAULT_D "Micro (hat) faults (summand 1/2 of 'mf' in vmstat and 'minf' in mpstat)"
#define SOLMEX_VM_HAT_FAULT_T "counter"
#define SOLMEX_VM_HAT_FAULT_N solmex_node_vm_hat_fault

#define SOLMEX_VM_KERNEL_ASFLT_D "Minor (as) faults in kernel addr space"
#define SOLMEX_VM_KERNEL_ASFLT_T "counter"
#define SOLMEX_VM_KERNEL_ASFLT_N solmex_node_vm_kernel_asflt

#define SOLMEX_VM_MAJ_FAULT_D "Major faults ('mjf' in mpstat)"
#define SOLMEX_VM_MAJ_FAULT_T "counter"
#define SOLMEX_VM_MAJ_FAULT_N solmex_node_vm_maj_fault

#define SOLMEX_VM_PGFREC_D "Page reclaims from free list"
#define SOLMEX_VM_PGFREC_T "counter"
#define SOLMEX_VM_PGFREC_N solmex_node_vm_pgfrec

#define SOLMEX_VM_PGIN_D "page ins ('pi' in vmstat)"
#define SOLMEX_VM_PGIN_T "counter"
#define SOLMEX_VM_PGIN_N solmex_node_vm_pgin

#define SOLMEX_VM_PGOUT_D "page outs ('po' in vmstat)"
#define SOLMEX_VM_PGOUT_T "counter"
#define SOLMEX_VM_PGOUT_N solmex_node_vm_pgout

#define SOLMEX_VM_PGPGIN_D "Pages paged in"
#define SOLMEX_VM_PGPGIN_T "counter"
#define SOLMEX_VM_PGPGIN_N solmex_node_vm_pgpgin

#define SOLMEX_VM_PGPGOUT_D "Pages paged out"
#define SOLMEX_VM_PGPGOUT_T "counter"
#define SOLMEX_VM_PGPGOUT_N solmex_node_vm_pgpgout

#define SOLMEX_VM_PGREC_D "Total page reclaims including pageouts ('re' in vmstat)"
#define SOLMEX_VM_PGREC_T "counter"
#define SOLMEX_VM_PGREC_N solmex_node_vm_pgrec

#define SOLMEX_VM_PGRRUN_D "times pager scheduled"
#define SOLMEX_VM_PGRRUN_T "counter"
#define SOLMEX_VM_PGRRUN_N solmex_node_vm_pgrrun

#define SOLMEX_VM_PGSWAPIN_D "pages swapped in"
#define SOLMEX_VM_PGSWAPIN_T "counter"
#define SOLMEX_VM_PGSWAPIN_N solmex_node_vm_pgswapin

#define SOLMEX_VM_PGSWAPOUT_D "pages swapped out"
#define SOLMEX_VM_PGSWAPOUT_T "counter"
#define SOLMEX_VM_PGSWAPOUT_N solmex_node_vm_pgswapout

#define SOLMEX_VM_PROT_FAULT_D "protection faults"
#define SOLMEX_VM_PROT_FAULT_T "counter"
#define SOLMEX_VM_PROT_FAULT_N solmex_node_vm_prot_fault

#define SOLMEX_VM_REV_D "revolutions of the page daemon hand"
#define SOLMEX_VM_REV_T "counter"
#define SOLMEX_VM_REV_N solmex_node_vm_rev

#define SOLMEX_VM_SCAN_D "pages examined by pageout daemon ('sr' in vmstat)"
#define SOLMEX_VM_SCAN_T "counter"
#define SOLMEX_VM_SCAN_N solmex_node_vm_scan

#define SOLMEX_VM_SOFTLOCK_D "faults due to software locking req"
#define SOLMEX_VM_SOFTLOCK_T "counter"
#define SOLMEX_VM_SOFTLOCK_N solmex_node_vm_softlock

#define SOLMEX_VM_SWAPIN_D "swapins ('si' in 'vmstat -S')"
#define SOLMEX_VM_SWAPIN_T "counter"
#define SOLMEX_VM_SWAPIN_N solmex_node_vm_swapin

#define SOLMEX_VM_SWAPOUT_D "swapouts ('so' in 'vmstat -S')"
#define SOLMEX_VM_SWAPOUT_T "counter"
#define SOLMEX_VM_SWAPOUT_N solmex_node_vm_swapout

#define SOLMEX_VM_ZFOD_D "pages zero filled on demand"
#define SOLMEX_VM_ZFOD_T "counter"
#define SOLMEX_VM_ZFOD_N solmex_node_vm_zfod


// kstat cpu::sys selection
#define SOLMEX_SYS_NAME_PREFIX "solmex_node_sys_"

#define SOLMEX_SYS_CPU_TICKS_IDLE_D "idle ticks ('id' in vmstat, 'idl' in mpstat as %)"
#define SOLMEX_SYS_CPU_TICKS_IDLE_T "counter"
#define SOLMEX_SYS_CPU_TICKS_IDLE_N solmex_node_sys_cpu_idle_ticks

#define SOLMEX_SYS_CPU_TICKS_KERNEL_D "kernel ticks ('cpu sy' in vmstat, 'sys' in mpstat as %)"
#define SOLMEX_SYS_CPU_TICKS_KERNEL_T "counter"
#define SOLMEX_SYS_CPU_TICKS_KERNEL_N solmex_node_sys_cpu_kernel_ticks

#define SOLMEX_SYS_CPU_TICKS_USER_D "user ticks ('us' in vmstat, 'usr' in mpstat as %)"
#define SOLMEX_SYS_CPU_TICKS_USER_T "counter"
#define SOLMEX_SYS_CPU_TICKS_USER_N solmex_node_sys_cpu_user_ticks

#define SOLMEX_SYS_CPU_TICKS_WAIT_D "waiting ticks ('st' in mpstat)"
#define SOLMEX_SYS_CPU_TICKS_WAIT_T "counter"
#define SOLMEX_SYS_CPU_TICKS_WAIT_N solmex_node_sys_cpu_wait_ticks

#define SOLMEX_SYS_INTR_D "device interrupts per PIL (PIL_MAX) array ('in' in vmstat, 'intr' in mpstat)"
#define SOLMEX_SYS_INTR_T "counter"
#define SOLMEX_SYS_INTR_N solmex_node_sys_intr

#define SOLMEX_SYS_INV_SWTCH_D "involuntary context switches ('icsw' in mpstat)"
#define SOLMEX_SYS_INV_SWTCH_T "counter"
#define SOLMEX_SYS_INV_SWTCH_N solmex_node_sys_inv_swtch

#define SOLMEX_SYS_NAMEI_D "pathname lookups"
#define SOLMEX_SYS_NAMEI_T "counter"
#define SOLMEX_SYS_NAMEI_N solmex_node_sys_namei

#define SOLMEX_SYS_NTHREADS_D "thread_create()s"
#define SOLMEX_SYS_NTHREADS_T "counter"
#define SOLMEX_SYS_NTHREADS_N solmex_node_sys_nthreads

#define SOLMEX_SYS_PHREAD_D "raw I/O reads"
#define SOLMEX_SYS_PHREAD_T "counter"
#define SOLMEX_SYS_PHREAD_N solmex_node_sys_phread

#define SOLMEX_SYS_PHWRITE_D "raw I/O writes"
#define SOLMEX_SYS_PHWRITE_T "counter"
#define SOLMEX_SYS_PHWRITE_N solmex_node_sys_phwrite

#define SOLMEX_SYS_PSWITCH_D "context switches ('cs' in vmstat, 'csw' in mpstat)"
#define SOLMEX_SYS_PSWITCH_T "counter"
#define SOLMEX_SYS_PSWITCH_N solmex_node_sys_pswitch

#define SOLMEX_SYS_SYSCALL_D "system calls ('faults sy' in vmstat, 'syscl' in mpstat)"
#define SOLMEX_SYS_SYSCALL_T "counter"
#define SOLMEX_SYS_SYSCALL_N solmex_node_sys_syscall

#define SOLMEX_SYS_SYSEXEC_D "execs"
#define SOLMEX_SYS_SYSEXEC_T "counter"
#define SOLMEX_SYS_SYSEXEC_N solmex_node_sys_sysexec

#define SOLMEX_SYS_SYSFORK_D "forks"
#define SOLMEX_SYS_SYSFORK_T "counter"
#define SOLMEX_SYS_SYSFORK_N solmex_node_sys_sysfork

#define SOLMEX_SYS_SYSREAD_D "read() + readv() system calls"
#define SOLMEX_SYS_SYSREAD_T "counter"
#define SOLMEX_SYS_SYSREAD_N solmex_node_sys_sysread

#define SOLMEX_SYS_SYSVFORK_D "vforks"
#define SOLMEX_SYS_SYSVFORK_T "counter"
#define SOLMEX_SYS_SYSVFORK_N solmex_node_sys_sysvfork

#define SOLMEX_SYS_SYSWRITE_D "write() + writev() system calls"
#define SOLMEX_SYS_SYSWRITE_T "counter"
#define SOLMEX_SYS_SYSWRITE_N solmex_node_sys_syswrite

#define SOLMEX_SYS_TRAP_D "traps"
#define SOLMEX_SYS_TRAP_T "counter"
#define SOLMEX_SYS_TRAP_N solmex_node_sys_trap

#define SOLMEX_SYS_XCALLS_D "cross-calls to other cpus ('xcal' in mpstat)"
#define SOLMEX_SYS_XCALLS_T "counter"
#define SOLMEX_SYS_XCALLS_N solmex_node_sys_xcalls

#define SOLMEX_SYS_IRQTHREAD_D "Interrupts as threads (not counting clock interrupt) ('ithr' in mpstat)"
#define SOLMEX_SYS_IRQTHREAD_T "counter"
#define SOLMEX_SYS_IRQTHREAD_N solmex_node_sys_intrthread

#define SOLMEX_SYS_MIGRATE_D "Thread migrations (to another processor) ('migr' in mpstat)"
#define SOLMEX_SYS_MIGRATE_T "counter"
#define SOLMEX_SYS_MIGRATE_N solmex_node_sys_cpumigrate

#define SOLMEX_SYS_MUTEXIN_D "Spins on mutexes (lock not acquired on first try) ('smtx' in mpstat)"
#define SOLMEX_SYS_MUTEXIN_T "counter"
#define SOLMEX_SYS_MUTEXIN_N solmex_node_sys_mutex_adenters

#define SOLMEX_SYS_RDFAILS_D "Spins on readers locks (lock not acquired on first try) (summand 1/2 of 'srw' in mpstat)"
#define SOLMEX_SYS_RDFAILS_T "counter"
#define SOLMEX_SYS_RDFAILS_N solmex_node_sys_rw_rdfails

#define SOLMEX_SYS_WRFAILS_D "Spins on writer locks (lock not acquired on first try)  (summand 2/2 of 'srw' in mpstat)"
#define SOLMEX_SYS_WRFAILS_T "counter"
#define SOLMEX_SYS_WRFAILS_N solmex_node_sys_rw_wrfails


// see  etc/kstat2solmex.sh -h
#define SOLMEX_NET_NAME_PREFIX "solmex_node_net_"

#define SOLMEX_NET_IFSPEED_BPS_D "Estimated bandwidth of the interface in bits per second"
#define SOLMEX_NET_IFSPEED_BPS_T "counter"
#define SOLMEX_NET_IFSPEED_BPS_N solmex_node_net_ifspeed_bps

#define SOLMEX_NET_LINK_STATE_D "Link state of the interface (1..up, 0..down)"
#define SOLMEX_NET_LINK_STATE_T "gauge"
#define SOLMEX_NET_LINK_STATE_N solmex_node_net_link_state

#define SOLMEX_NET_LRO_BADIPCSUMS_PKTS_D "Large Receive Offload: pakets with a bad IP checksums"
#define SOLMEX_NET_LRO_BADIPCSUMS_PKTS_T "counter"
#define SOLMEX_NET_LRO_BADIPCSUMS_PKTS_N solmex_node_net_lro_badipcsums_pkts

#define SOLMEX_NET_LRO_BADTCPCSUMS_PKTS_D "Large Receive Offload: pakets with a bad TCP checksums"
#define SOLMEX_NET_LRO_BADTCPCSUMS_PKTS_T "counter"
#define SOLMEX_NET_LRO_BADTCPCSUMS_PKTS_N solmex_node_net_lro_badtcpcsums_pkts

#define SOLMEX_NET_LRO_OUTSEQ_PKTS_D "Large Receive Offload: out of sequence pakets"
#define SOLMEX_NET_LRO_OUTSEQ_PKTS_T "counter"
#define SOLMEX_NET_LRO_OUTSEQ_PKTS_N solmex_node_net_lro_outseq_pkts

#define SOLMEX_NET_LRO_TRUNC_PKTS_D "Large Receive Offload: truncated pakets"
#define SOLMEX_NET_LRO_TRUNC_PKTS_T "counter"
#define SOLMEX_NET_LRO_TRUNC_PKTS_N solmex_node_net_lro_trunc_pkts

#define SOLMEX_NET_PHYS_STATE_D "State of the physical link (1..sleep,2..polling,3..disabled,4..training,5..up,6..recovery,7..test)"
#define SOLMEX_NET_PHYS_STATE_T "gauge"
#define SOLMEX_NET_PHYS_STATE_N solmex_node_net_phys_state

#define SOLMEX_NET_RX_BROADCAST_BYTES_D "Broadcast bytes successfully received"
#define SOLMEX_NET_RX_BROADCAST_BYTES_T "counter"
#define SOLMEX_NET_RX_BROADCAST_BYTES_N solmex_node_net_rx_broadcast_bytes

#define SOLMEX_NET_RX_BROADCAST_PKTS_D "Broadcast packets successfully received"
#define SOLMEX_NET_RX_BROADCAST_PKTS_T "counter"
#define SOLMEX_NET_RX_BROADCAST_PKTS_N solmex_node_net_rx_broadcast_pkts

#define SOLMEX_NET_RX_BYTES_D "Total bytes successfully received on the interface."
#define SOLMEX_NET_RX_BYTES_T "counter"
#define SOLMEX_NET_RX_BYTES_N solmex_node_net_rx_bytes

#define SOLMEX_NET_RX_BYTES64_D "Total bytes successfully received on the interface"
#define SOLMEX_NET_RX_BYTES64_T "counter"
#define SOLMEX_NET_RX_BYTES64_N solmex_node_net_rx_bytes

#define SOLMEX_NET_RX_DROP_BYTES_D "Incoming bytes dropped per datalink in the software"
#define SOLMEX_NET_RX_DROP_BYTES_T "counter"
#define SOLMEX_NET_RX_DROP_BYTES_N solmex_node_net_rx_drop_bytes

#define SOLMEX_NET_RX_DROP_PKTS_D "Incoming packets dropped per datalink in the software"
#define SOLMEX_NET_RX_DROP_PKTS_T "counter"
#define SOLMEX_NET_RX_DROP_PKTS_N solmex_node_net_rx_drop_pkts

#define SOLMEX_NET_RX_ERROR_PKTS_D "Total packets received that couldn't be processed because they contained errors"
#define SOLMEX_NET_RX_ERROR_PKTS_T "counter"
#define SOLMEX_NET_RX_ERROR_PKTS_N solmex_node_net_rx_error_pkts

#define SOLMEX_NET_RX_INTRS_D "Times interrupt handler was called and claimed the interrupt"
#define SOLMEX_NET_RX_INTRS_T "counter"
#define SOLMEX_NET_RX_INTRS_N solmex_node_net_rx_intrs

#define SOLMEX_NET_RX_INTRS_BYTES_D "Bytes processed when the interrupt handler was called and claimed the interrupt"
#define SOLMEX_NET_RX_INTRS_BYTES_T "counter"
#define SOLMEX_NET_RX_INTRS_BYTES_N solmex_node_net_rx_intrs_bytes

#define SOLMEX_NET_RX_LOCAL_BYTES_D "Bytes transmitted via a loop back interface"
#define SOLMEX_NET_RX_LOCAL_BYTES_T "counter"
#define SOLMEX_NET_RX_LOCAL_BYTES_N solmex_node_net_rx_local_bytes

#define SOLMEX_NET_RX_LOCAL_PKTS_D "Pakets received via a loop back interface"
#define SOLMEX_NET_RX_LOCAL_PKTS_T "counter"
#define SOLMEX_NET_RX_LOCAL_PKTS_N solmex_node_net_rx_local_pkts

#define SOLMEX_NET_RX_MULTICAST_BYTES_D "Multicast bytes successfully received, including group and functional addresses"
#define SOLMEX_NET_RX_MULTICAST_BYTES_T "counter"
#define SOLMEX_NET_RX_MULTICAST_BYTES_N solmex_node_net_rx_multicast_bytes

#define SOLMEX_NET_RX_MULTICAST_PKTS_D "Multicast packets successfully received, including group and functional addresses"
#define SOLMEX_NET_RX_MULTICAST_PKTS_T "counter"
#define SOLMEX_NET_RX_MULTICAST_PKTS_N solmex_node_net_rx_multicast_pkts

#define SOLMEX_NET_RX_NOBUF_PKTS_D "Times a valid incoming packet was known to have been discarded because no buffer could be allocated for receive"
#define SOLMEX_NET_RX_NOBUF_PKTS_T "counter"
#define SOLMEX_NET_RX_NOBUF_PKTS_N solmex_node_net_rx_nobuf_pkts

#define SOLMEX_NET_RX_PKTS_D "Total packets successfully received on the interface"
#define SOLMEX_NET_RX_PKTS_T "counter"
#define SOLMEX_NET_RX_PKTS_N solmex_node_net_rx_pkts

#define SOLMEX_NET_RX_PKTS64_D "Total packets successfully received on the interface"
#define SOLMEX_NET_RX_PKTS64_T "counter"
#define SOLMEX_NET_RX_PKTS64_N solmex_node_net_rx_pkts

#define SOLMEX_NET_RX_POLLS_D "hardware polls"
#define SOLMEX_NET_RX_POLLS_T "counter"
#define SOLMEX_NET_RX_POLLS_N solmex_node_net_rx_polls

#define SOLMEX_NET_RX_POLLS_BYTES_D "bytes processed via hardware polls"
#define SOLMEX_NET_RX_POLLS_BYTES_T "counter"
#define SOLMEX_NET_RX_POLLS_BYTES_N solmex_node_net_rx_polls_bytes

#define SOLMEX_NET_SEC_DHCPSPOOFED_PKTS_D "Spoofed DHCP pakets seen"
#define SOLMEX_NET_SEC_DHCPSPOOFED_PKTS_T "counter"
#define SOLMEX_NET_SEC_DHCPSPOOFED_PKTS_N solmex_node_net_sec_dhcpspoofed_pkts

#define SOLMEX_NET_SEC_DHCPDROPPED_PKTS_D "Spoofed DHCP pakets dropped"
#define SOLMEX_NET_SEC_DHCPDROPPED_PKTS_T "counter"
#define SOLMEX_NET_SEC_DHCPDROPPED_PKTS_N solmex_node_net_sec_dhcpdropped_pkts

#define SOLMEX_NET_SEC_IPSPOOFED_PKTS_D "Pakets with a spoofed IP dropped"
#define SOLMEX_NET_SEC_IPSPOOFED_PKTS_T "counter"
#define SOLMEX_NET_SEC_IPSPOOFED_PKTS_N solmex_node_net_sec_ipspoofed_pkts

#define SOLMEX_NET_SEC_MACSPOOFED_PKTS_D "Pakets with a poofed MAC address dropped"
#define SOLMEX_NET_SEC_MACSPOOFED_PKTS_T "counter"
#define SOLMEX_NET_SEC_MACSPOOFED_PKTS_N solmex_node_net_sec_macspoofed_pkts

#define SOLMEX_NET_SEC_RESTRICTED_PKTS_D "Pakets dropped, which are neither IP nor ARP pakets"
#define SOLMEX_NET_SEC_RESTRICTED_PKTS_T "counter"
#define SOLMEX_NET_SEC_RESTRICTED_PKTS_N solmex_node_net_sec_restricted_pkts

#define SOLMEX_NET_TX_BLOCK_PKTS_D "Pakets received which could not be put up a stream because the queue was flow controlled (misc tx stats: times blocked for Tx descs)"
#define SOLMEX_NET_TX_BLOCK_PKTS_T "counter"
#define SOLMEX_NET_TX_BLOCK_PKTS_N solmex_node_net_tx_block_pkts

#define SOLMEX_NET_TX_BROADCAST_BYTES_D "Broadcast bytes requested to be transmitted"
#define SOLMEX_NET_TX_BROADCAST_BYTES_T "counter"
#define SOLMEX_NET_TX_BROADCAST_BYTES_N solmex_node_net_tx_broadcast_bytes

#define SOLMEX_NET_TX_BROADCAST_PKTS_D "Broadcast packets requested to be transmitted"
#define SOLMEX_NET_TX_BROADCAST_PKTS_T "counter"
#define SOLMEX_NET_TX_BROADCAST_PKTS_N solmex_node_net_tx_broadcast_pkts

#define SOLMEX_NET_TX_BYTES_D "Total bytes requested to be transmitted on the interface"
#define SOLMEX_NET_TX_BYTES_T "counter"
#define SOLMEX_NET_TX_BYTES_N solmex_node_net_tx_bytes

#define SOLMEX_NET_TX_BYTES64_D "Total bytes requested to be transmitted on the interface"
#define SOLMEX_NET_TX_BYTES64_T "counter"
#define SOLMEX_NET_TX_BYTES64_N solmex_node_net_tx_bytes

#define SOLMEX_NET_TX_COLLS_D "Ethernet collisions during transmit"
#define SOLMEX_NET_TX_COLLS_T "counter"
#define SOLMEX_NET_TX_COLLS_N solmex_node_net_tx_colls

#define SOLMEX_NET_TX_DROP_BYTES_D "Outgoing bytes dropped per datalink in the software"
#define SOLMEX_NET_TX_DROP_BYTES_T "counter"
#define SOLMEX_NET_TX_DROP_BYTES_N solmex_node_net_tx_drop_bytes

#define SOLMEX_NET_TX_DROP_PKTS_D "Outgoing packets dropped per datalink in the software"
#define SOLMEX_NET_TX_DROP_PKTS_T "counter"
#define SOLMEX_NET_TX_DROP_PKTS_N solmex_node_net_tx_drop_pkts

#define SOLMEX_NET_TX_ERROR_PKTS_D "Total packets that weren't successfully transmitted because of errors"
#define SOLMEX_NET_TX_ERROR_PKTS_T "counter"
#define SOLMEX_NET_TX_ERROR_PKTS_N solmex_node_net_tx_error_pkts

#define SOLMEX_NET_TX_LOCAL_BYTES_D "Bytes transmitted via a loopback interface"
#define SOLMEX_NET_TX_LOCAL_BYTES_T "counter"
#define SOLMEX_NET_TX_LOCAL_BYTES_N solmex_node_net_tx_local_bytes

#define SOLMEX_NET_TX_LOCAL_PKTS_D "Pakets transmitted via a loop back interface"
#define SOLMEX_NET_TX_LOCAL_PKTS_T "counter"
#define SOLMEX_NET_TX_LOCAL_PKTS_N solmex_node_net_tx_local_pkts

#define SOLMEX_NET_TX_MISC_ERROR_PKTS_D "Mac misc stats: vid_check, tag needed errors"
#define SOLMEX_NET_TX_MISC_ERROR_PKTS_T "counter"
#define SOLMEX_NET_TX_MISC_ERROR_PKTS_N solmex_node_net_tx_misc_error_pkts

#define SOLMEX_NET_TX_MULTICAST_BYTES_D "Multicast bytes requested to be transmitted, including group and functional addresses"
#define SOLMEX_NET_TX_MULTICAST_BYTES_T "counter"
#define SOLMEX_NET_TX_MULTICAST_BYTES_N solmex_node_net_tx_multicast_bytes

#define SOLMEX_NET_TX_MULTICAST_PKTS_D "Multicast packets requested to be transmitted, including group and functional addresses"
#define SOLMEX_NET_TX_MULTICAST_PKTS_T "counter"
#define SOLMEX_NET_TX_MULTICAST_PKTS_N solmex_node_net_tx_multicast_pkts

#define SOLMEX_NET_TX_NOBUF_PKTS_D "Packets discarded on output because transmit buffer was busy, or no buffer could be allocated for transmit"
#define SOLMEX_NET_TX_NOBUF_PKTS_T "counter"
#define SOLMEX_NET_TX_NOBUF_PKTS_N solmex_node_net_tx_nobuf_pkts

#define SOLMEX_NET_TX_PKTS_D "Total packets requested to be transmitted on the interface"
#define SOLMEX_NET_TX_PKTS_T "counter"
#define SOLMEX_NET_TX_PKTS_N solmex_node_net_tx_pkts

#define SOLMEX_NET_TX_PKTS64_D "Total packets requested to be transmitted on the interface"
#define SOLMEX_NET_TX_PKTS64_T "counter"
#define SOLMEX_NET_TX_PKTS64_N solmex_node_net_tx_pkts

#define SOLMEX_NET_TX_UNBLOCK_CALLS_D "misc tx stats: unblock calls from driver"
#define SOLMEX_NET_TX_UNBLOCK_CALLS_T "counter"
#define SOLMEX_NET_TX_UNBLOCK_CALLS_N solmex_node_net_tx_unblock_calls


/*
#define SOLMEXM_XXX_D "short description."
#define SOLMEXM_XXX_T "gauge"
#define SOLMEXM_XXX_N "solmex_yyy"
 */

#ifdef __cplusplus
}
#endif

#endif // SOLMEX_COMMON_H
