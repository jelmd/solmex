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

/* mib2 class */
#define SOLMEX_MIB_NAME_PREFIX solmex_node_netstat_


#define SOLMEX_RAWIP_INCKSUMERRS_D "Number of raw IP packets with bad IPV6_CHECKSUM checksums"
#define SOLMEX_RAWIP_INCKSUMERRS_T "counter"
#define SOLMEX_RAWIP_INCKSUMERRS_N solmex_node_netstat_Rawip_InCksumErrs

#define SOLMEX_RAWIP_INDATAGRAMS_D "Number of raw IP datagrams sent upstream"
#define SOLMEX_RAWIP_INDATAGRAMS_T "counter"
#define SOLMEX_RAWIP_INDATAGRAMS_N solmex_node_netstat_Rawip_InDatagrams

#define SOLMEX_RAWIP_INERRORS_D "Number of received datagrams not deliverable (other)"
#define SOLMEX_RAWIP_INERRORS_T "counter"
#define SOLMEX_RAWIP_INERRORS_N solmex_node_netstat_Rawip_InErrors

#define SOLMEX_RAWIP_OUTDATAGRAMS_D "Number of datagrams sent"
#define SOLMEX_RAWIP_OUTDATAGRAMS_T "counter"
#define SOLMEX_RAWIP_OUTDATAGRAMS_N solmex_node_netstat_Rawip_OutDatagrams

#define SOLMEX_RAWIP_OUTERRORS_D "Number of datagrams not sent (e.g. no memory)"
#define SOLMEX_RAWIP_OUTERRORS_T "counter"
#define SOLMEX_RAWIP_OUTERRORS_N solmex_node_netstat_Rawip_OutErrors

// (#) .. ipIfStatsEntry #, (sys #) .. ipSystemStatsEntry #,
#define SOLMEX_IP_ADDRENTRYSIZE_D "Size of a MIB2 ip address information entry associated with the related NIC in bytes - (ip 20)"
#define SOLMEX_IP_ADDRENTRYSIZE_T "counter"
#define SOLMEX_IP_ADDRENTRYSIZE_N solmex_node_netstat_Ip_AddrEntrySize

#define SOLMEX_IP_DEFAULTTTL_D "Default time to live value (or hop limit for IPv6)"
#define SOLMEX_IP_DEFAULTTTL_T "counter"
#define SOLMEX_IP_DEFAULTTTL_N solmex_node_netstat_Ip_DefaultTTL

#define SOLMEX_IP_FORWDATAGRAMS_D "Forwarded datagams sent out - (23)"
#define SOLMEX_IP_FORWDATAGRAMS_T "counter"
#define SOLMEX_IP_FORWDATAGRAMS_N solmex_node_netstat_Ip_ForwDatagrams

#define SOLMEX_IP_FORWPROHIBITS_D "IP packets not forwarded due to adminstrative reasons"
#define SOLMEX_IP_FORWPROHIBITS_T "counter"
#define SOLMEX_IP_FORWPROHIBITS_N solmex_node_netstat_Ip_ForwProhibits

#define SOLMEX_IP_FORWARDING_D "IP forwarder status: 1 gateway, 2 NOT gateway"
#define SOLMEX_IP_FORWARDING_T "counter"
#define SOLMEX_IP_FORWARDING_N solmex_node_netstat_Ip_Forwarding

#define SOLMEX_IP_FRAGCREATES_D "IP packet fragments created - (29)"
#define SOLMEX_IP_FRAGCREATES_T "counter"
#define SOLMEX_IP_FRAGCREATES_N solmex_node_netstat_Ip_FragCreates

#define SOLMEX_IP_FRAGFAILS_D "IP packets dropped because its fragmentation failed - (28)"
#define SOLMEX_IP_FRAGFAILS_T "counter"
#define SOLMEX_IP_FRAGFAILS_N solmex_node_netstat_Ip_FragFails

#define SOLMEX_IP_FRAGOKS_D "Sucessfully fragmented IP packets - (27)"
#define SOLMEX_IP_FRAGOKS_T "counter"
#define SOLMEX_IP_FRAGOKS_N solmex_node_netstat_Ip_FragOKs

#define SOLMEX_IP_INADDRERRORS_D "IP pakets with an invalid or unsupported address - (9)"
#define SOLMEX_IP_INADDRERRORS_T "counter"
#define SOLMEX_IP_INADDRERRORS_N solmex_node_netstat_Ip_InAddrErrors

#define SOLMEX_IP_INCKSUMERRS_D "IP packets with a bad IP header checksums"
#define SOLMEX_IP_INCKSUMERRS_T "counter"
#define SOLMEX_IP_INCKSUMERRS_N solmex_node_netstat_Ip_InCksumErrs

#define SOLMEX_IP_INDELIVERS_D "IP pakets delivered to upper layer protocols - (19)"
#define SOLMEX_IP_INDELIVERS_T "counter"
#define SOLMEX_IP_INDELIVERS_N solmex_node_netstat_Ip_InDelivers

#define SOLMEX_IP_INDISCARDS_D "IP pakets discarded e.g. due to no buffers - (17)"
#define SOLMEX_IP_INDISCARDS_T "counter"
#define SOLMEX_IP_INDISCARDS_N solmex_node_netstat_Ip_InDiscards

#define SOLMEX_IP_INERRS_D "TCP packets with a bad TCP checksum - (tcp 14)"
#define SOLMEX_IP_INERRS_T "counter"
#define SOLMEX_IP_INERRS_N solmex_node_netstat_Ip_InErrs

#define SOLMEX_IP_INHDRERRORS_D "IP packets with errors in IP headers and options - (7)"
#define SOLMEX_IP_INHDRERRORS_T "counter"
#define SOLMEX_IP_INHDRERRORS_N solmex_node_netstat_Ip_InHdrErrors

#define SOLMEX_IP_INIPV6_D "IPv6 packets received and dropped by IPv4"
#define SOLMEX_IP_INIPV6_T "counter"
#define SOLMEX_IP_INIPV6_N solmex_node_netstat_Ip_InIPv6

#define SOLMEX_IP_INRECEIVES_D "Received IP pakets (incl errors) - (3)"
#define SOLMEX_IP_INRECEIVES_T "counter"
#define SOLMEX_IP_INRECEIVES_N solmex_node_netstat_Ip_InReceives

#define SOLMEX_IP_INUNKNOWNPROTOS_D "IP packets with an unknown next header - (10)"
#define SOLMEX_IP_INUNKNOWNPROTOS_T "counter"
#define SOLMEX_IP_INUNKNOWNPROTOS_N solmex_node_netstat_Ip_InUnknownProtos

#define SOLMEX_IP_IPSECINFAILED_D "Received IP packets that not succeeded policy checks"
#define SOLMEX_IP_IPSECINFAILED_T "counter"
#define SOLMEX_IP_IPSECINFAILED_N solmex_node_netstat_Ip_IpsecInFailed

#define SOLMEX_IP_IPSECINSUCCEEDED_D "Received IP packets that succeeded policy checks"
#define SOLMEX_IP_IPSECINSUCCEEDED_T "counter"
#define SOLMEX_IP_IPSECINSUCCEEDED_N solmex_node_netstat_Ip_IpsecInSucceeded

#define SOLMEX_IP_MEMBERENTRYSIZE_D "Size of a MIB2 group members list entry for an interface in bytes"
#define SOLMEX_IP_MEMBERENTRYSIZE_T "counter"
#define SOLMEX_IP_MEMBERENTRYSIZE_N solmex_node_netstat_Ip_MemberEntrySize

#define SOLMEX_IP_NETTOMEDIAENTRYSIZE_D "Size of a MIB2 ip to physical address entry in bytes - (ip 22)"
#define SOLMEX_IP_NETTOMEDIAENTRYSIZE_T "counter"
#define SOLMEX_IP_NETTOMEDIAENTRYSIZE_N solmex_node_netstat_Ip_NetToMediaEntrySize

#define SOLMEX_IP_NOPORTS_D "Received IP packets not deliverable (no appl.) - (udp 2)"
#define SOLMEX_IP_NOPORTS_T "counter"
#define SOLMEX_IP_NOPORTS_N solmex_node_netstat_Ip_NoPorts

#define SOLMEX_IP_OUTDISCARDS_D "IP packets discarded e.g. due to no buffers - (25)"
#define SOLMEX_IP_OUTDISCARDS_T "counter"
#define SOLMEX_IP_OUTDISCARDS_N solmex_node_netstat_Ip_OutDiscards

#define SOLMEX_IP_OUTIPV6_D "no longer used"
#define SOLMEX_IP_OUTIPV6_T "counter"
#define SOLMEX_IP_OUTIPV6_N solmex_node_netstat_Ip_OutIPv6

#define SOLMEX_IP_OUTNOROUTES_D "IP packets discarded due to no route to dest - (sys 22)"
#define SOLMEX_IP_OUTNOROUTES_T "counter"
#define SOLMEX_IP_OUTNOROUTES_N solmex_node_netstat_Ip_OutNoRoutes

#define SOLMEX_IP_OUTREQUESTS_D "IP packets sent out (includes dropped/discarded packets) - (21)"
#define SOLMEX_IP_OUTREQUESTS_T "counter"
#define SOLMEX_IP_OUTREQUESTS_N solmex_node_netstat_Ip_OutRequests

#define SOLMEX_IP_OUTSWITCHIPV6_D "no longer used"
#define SOLMEX_IP_OUTSWITCHIPV6_T "counter"
#define SOLMEX_IP_OUTSWITCHIPV6_N solmex_node_netstat_Ip_OutSwitchIPv6

#define SOLMEX_IP_RAWIPINOVERFLOWS_D "Raw IP packets (all IP protocols except UDP, TCP and ICMP) dropped due to queue overflow"
#define SOLMEX_IP_RAWIPINOVERFLOWS_T "counter"
#define SOLMEX_IP_RAWIPINOVERFLOWS_N solmex_node_netstat_Ip_RawipInOverflows

#define SOLMEX_IP_REASMDUPLICATES_D "Complete duplicates in reassembly"
#define SOLMEX_IP_REASMDUPLICATES_T "counter"
#define SOLMEX_IP_REASMDUPLICATES_N solmex_node_netstat_Ip_ReasmDuplicates

#define SOLMEX_IP_REASMFAILS_D "IP packets for which reassembly failed - (16)"
#define SOLMEX_IP_REASMFAILS_T "counter"
#define SOLMEX_IP_REASMFAILS_N solmex_node_netstat_Ip_ReasmFails

#define SOLMEX_IP_REASMOKS_D "Reassembled IP packets - (15)"
#define SOLMEX_IP_REASMOKS_T "counter"
#define SOLMEX_IP_REASMOKS_N solmex_node_netstat_Ip_ReasmOKs

#define SOLMEX_IP_REASMPARTDUPS_D "Partial duplicates in IP packet reassembly"
#define SOLMEX_IP_REASMPARTDUPS_T "counter"
#define SOLMEX_IP_REASMPARTDUPS_N solmex_node_netstat_Ip_ReasmPartDups

#define SOLMEX_IP_REASMREQDS_D "Received IP packet fragments - (14)"
#define SOLMEX_IP_REASMREQDS_T "counter"
#define SOLMEX_IP_REASMREQDS_N solmex_node_netstat_Ip_ReasmReqds

#define SOLMEX_IP_REASMTIMEOUT_D "Timeout in seconds for reassembling an IP packet"
#define SOLMEX_IP_REASMTIMEOUT_T "counter"
#define SOLMEX_IP_REASMTIMEOUT_N solmex_node_netstat_Ip_ReasmTimeout

#define SOLMEX_IP_ROUTEENTRYSIZE_D "Size of an MIB2 ip route entry in bytes - (ip 21)"
#define SOLMEX_IP_ROUTEENTRYSIZE_T "counter"
#define SOLMEX_IP_ROUTEENTRYSIZE_N solmex_node_netstat_Ip_RouteEntrySize

#define SOLMEX_IP_ROUTINGDISCARDS_D "not used - always 0"
#define SOLMEX_IP_ROUTINGDISCARDS_T "counter"
#define SOLMEX_IP_ROUTINGDISCARDS_N solmex_node_netstat_Ip_RoutingDiscards

#define SOLMEX_IP_UDPINCKSUMERRS_D "UDP packets with a bad UDP checksum"
#define SOLMEX_IP_UDPINCKSUMERRS_T "counter"
#define SOLMEX_IP_UDPINCKSUMERRS_N solmex_node_netstat_Ip_UdpInCksumErrs

#define SOLMEX_IP_UDPINOVERFLOWS_D "UDP packets dropped due to queue overflow"
#define SOLMEX_IP_UDPINOVERFLOWS_T "counter"
#define SOLMEX_IP_UDPINOVERFLOWS_N solmex_node_netstat_Ip_UdpInOverflows

// icmp
#define SOLMEX_ICMP_INADDRMASKREPS_D "Received ICMP address mask request replies (18)"
#define SOLMEX_ICMP_INADDRMASKREPS_T "counter"
#define SOLMEX_ICMP_INADDRMASKREPS_N solmex_node_netstat_Icmp_InAddrMaskReps

#define SOLMEX_ICMP_INADDRMASKS_D "Received ICMP address mask request messages (17, if honored by the OS policy)"
#define SOLMEX_ICMP_INADDRMASKS_T "counter"
#define SOLMEX_ICMP_INADDRMASKS_N solmex_node_netstat_Icmp_InAddrMasks

#define SOLMEX_ICMP_INBADREDIRECTS_D "Received ICMP redirect messages dropped because of a wrong/un-allowed destination or unknown code"
#define SOLMEX_ICMP_INBADREDIRECTS_T "counter"
#define SOLMEX_ICMP_INBADREDIRECTS_N solmex_node_netstat_Icmp_InBadRedirects

#define SOLMEX_ICMP_INCHKSUMERRS_D "Received ICMP messages with a checksum error"
#define SOLMEX_ICMP_INCHKSUMERRS_T "counter"
#define SOLMEX_ICMP_INCHKSUMERRS_N solmex_node_netstat_Icmp_InChksumErrs

#define SOLMEX_ICMP_INDESTUNREACHS_D "Received ICMP destionation unreachable messages (3, incl. fragmentation needed messages)"
#define SOLMEX_ICMP_INDESTUNREACHS_T "counter"
#define SOLMEX_ICMP_INDESTUNREACHS_N solmex_node_netstat_Icmp_InDestUnreachs

#define SOLMEX_ICMP_INECHOREPS_D "Received ICMP echo replay messages (0)"
#define SOLMEX_ICMP_INECHOREPS_T "counter"
#define SOLMEX_ICMP_INECHOREPS_N solmex_node_netstat_Icmp_InEchoReps

#define SOLMEX_ICMP_INECHOS_D "Received ICMP echo requests (8)"
#define SOLMEX_ICMP_INECHOS_T "counter"
#define SOLMEX_ICMP_INECHOS_N solmex_node_netstat_Icmp_InEchos

#define SOLMEX_ICMP_INERRORS_D "Received invalid ICMP messages (payload problem) or messages with un-allowed destination"
#define SOLMEX_ICMP_INERRORS_T "counter"
#define SOLMEX_ICMP_INERRORS_N solmex_node_netstat_Icmp_InErrors

#define SOLMEX_ICMP_INFRAGNEEDED_D "Received ICMP destionation unreachable - fragmentation needed messages (3:4)"
#define SOLMEX_ICMP_INFRAGNEEDED_T "counter"
#define SOLMEX_ICMP_INFRAGNEEDED_N solmex_node_netstat_Icmp_InFragNeeded

#define SOLMEX_ICMP_INMSGS_D "Received ICMP messages (incl. maleformed/truncated packets)"
#define SOLMEX_ICMP_INMSGS_T "counter"
#define SOLMEX_ICMP_INMSGS_N solmex_node_netstat_Icmp_InMsgs

#define SOLMEX_ICMP_INOVERFLOWS_D "ICMP messages which could not be delivered to the corresponding connection"
#define SOLMEX_ICMP_INOVERFLOWS_T "counter"
#define SOLMEX_ICMP_INOVERFLOWS_N solmex_node_netstat_Icmp_InOverFlows

#define SOLMEX_ICMP_INPARMPROBS_D "Received ICMP parameter problem messages (12)"
#define SOLMEX_ICMP_INPARMPROBS_T "counter"
#define SOLMEX_ICMP_INPARMPROBS_N solmex_node_netstat_Icmp_InParmProbs

#define SOLMEX_ICMP_INREDIRECTS_D "Received ICMP redirect messages (5)"
#define SOLMEX_ICMP_INREDIRECTS_T "counter"
#define SOLMEX_ICMP_INREDIRECTS_N solmex_node_netstat_Icmp_InRedirects

#define SOLMEX_ICMP_INSRCQUENCHS_D "Received ICMP source quench messages (4, congestion control)"
#define SOLMEX_ICMP_INSRCQUENCHS_T "counter"
#define SOLMEX_ICMP_INSRCQUENCHS_N solmex_node_netstat_Icmp_InSrcQuenchs

#define SOLMEX_ICMP_INTIMEEXCDS_D "Received ICMP time exceeded messages (11)"
#define SOLMEX_ICMP_INTIMEEXCDS_T "counter"
#define SOLMEX_ICMP_INTIMEEXCDS_N solmex_node_netstat_Icmp_InTimeExcds

#define SOLMEX_ICMP_INTIMESTAMPREPS_D "Received ICMP timestamp reply messages (14)"
#define SOLMEX_ICMP_INTIMESTAMPREPS_T "counter"
#define SOLMEX_ICMP_INTIMESTAMPREPS_N solmex_node_netstat_Icmp_InTimestampReps

#define SOLMEX_ICMP_INTIMESTAMPS_D "Received ICMP timestamp messages (13, if honored by the OS policy)"
#define SOLMEX_ICMP_INTIMESTAMPS_T "counter"
#define SOLMEX_ICMP_INTIMESTAMPS_N solmex_node_netstat_Icmp_InTimestamps

#define SOLMEX_ICMP_INUNKNOWNS_D "Received ICMP messages having an unknown type or code and thus not handled by the IP layer of the OS"
#define SOLMEX_ICMP_INUNKNOWNS_T "counter"
#define SOLMEX_ICMP_INUNKNOWNS_N solmex_node_netstat_Icmp_InUnknowns

#define SOLMEX_ICMP_OUTADDRMASKREPS_D "ICMP address mask request reply messages sent (18)"
#define SOLMEX_ICMP_OUTADDRMASKREPS_T "counter"
#define SOLMEX_ICMP_OUTADDRMASKREPS_N solmex_node_netstat_Icmp_OutAddrMaskReps

#define SOLMEX_ICMP_OUTADDRMASKS_D "ICMP address mask request messages sent (17) = 0"
#define SOLMEX_ICMP_OUTADDRMASKS_T "counter"
#define SOLMEX_ICMP_OUTADDRMASKS_N solmex_node_netstat_Icmp_OutAddrMasks

#define SOLMEX_ICMP_OUTDESTUNREACHS_D "ICMP destination unreachable messages sent (3, incl. fragmentation needed messages)"
#define SOLMEX_ICMP_OUTDESTUNREACHS_T "counter"
#define SOLMEX_ICMP_OUTDESTUNREACHS_N solmex_node_netstat_Icmp_OutDestUnreachs

#define SOLMEX_ICMP_OUTDROPS_D "ICMP replies not sent because of request copy problems, or error messages with a broad/multicast/un-allowed source or destination, or error messages in response to an ICMP error"
#define SOLMEX_ICMP_OUTDROPS_T "counter"
#define SOLMEX_ICMP_OUTDROPS_N solmex_node_netstat_Icmp_OutDrops

#define SOLMEX_ICMP_OUTECHOREPS_D "ICMP echo replies sent (0)"
#define SOLMEX_ICMP_OUTECHOREPS_T "counter"
#define SOLMEX_ICMP_OUTECHOREPS_N solmex_node_netstat_Icmp_OutEchoReps

#define SOLMEX_ICMP_OUTECHOS_D "ICMP address mask request replies sent (18)"
#define SOLMEX_ICMP_OUTECHOS_T "counter"
#define SOLMEX_ICMP_OUTECHOS_N solmex_node_netstat_Icmp_OutEchos

#define SOLMEX_ICMP_OUTERRORS_D "ICMP messages not sent because of memory allocation problems."
#define SOLMEX_ICMP_OUTERRORS_T "counter"
#define SOLMEX_ICMP_OUTERRORS_N solmex_node_netstat_Icmp_OutErrors

#define SOLMEX_ICMP_OUTFRAGNEEDED_D "ICMP destination unreachable - fragmentation needed messages sent (3:4)"
#define SOLMEX_ICMP_OUTFRAGNEEDED_T "counter"
#define SOLMEX_ICMP_OUTFRAGNEEDED_N solmex_node_netstat_Icmp_OutFragNeeded

#define SOLMEX_ICMP_OUTMSGS_D "ICMP messages sent"
#define SOLMEX_ICMP_OUTMSGS_T "counter"
#define SOLMEX_ICMP_OUTMSGS_N solmex_node_netstat_Icmp_OutMsgs

#define SOLMEX_ICMP_OUTPARMPROBS_D "ICMP parameter problem messages sent (12)"
#define SOLMEX_ICMP_OUTPARMPROBS_T "counter"
#define SOLMEX_ICMP_OUTPARMPROBS_N solmex_node_netstat_Icmp_OutParmProbs

#define SOLMEX_ICMP_OUTREDIRECTS_D "ICMP redirect messages sent (5)"
#define SOLMEX_ICMP_OUTREDIRECTS_T "counter"
#define SOLMEX_ICMP_OUTREDIRECTS_N solmex_node_netstat_Icmp_OutRedirects

#define SOLMEX_ICMP_OUTSRCQUENCHS_D "ICMP source quench messages sent (4) = 0"
#define SOLMEX_ICMP_OUTSRCQUENCHS_T "counter"
#define SOLMEX_ICMP_OUTSRCQUENCHS_N solmex_node_netstat_Icmp_OutSrcQuenchs

#define SOLMEX_ICMP_OUTTIMEEXCDS_D "ICMP time exceeded messages sent (11)"
#define SOLMEX_ICMP_OUTTIMEEXCDS_T "counter"
#define SOLMEX_ICMP_OUTTIMEEXCDS_N solmex_node_netstat_Icmp_OutTimeExcds

#define SOLMEX_ICMP_OUTTIMESTAMPREPS_D "ICMP timestamp reply messages sent (14)"
#define SOLMEX_ICMP_OUTTIMESTAMPREPS_T "counter"
#define SOLMEX_ICMP_OUTTIMESTAMPREPS_N solmex_node_netstat_Icmp_OutTimestampReps

#define SOLMEX_ICMP_OUTTIMESTAMPS_D "ICMP timestamp messages sent (13)"
#define SOLMEX_ICMP_OUTTIMESTAMPS_T "counter"
#define SOLMEX_ICMP_OUTTIMESTAMPS_N solmex_node_netstat_Icmp_OutTimestamps

// (#) .. sctpStats #, (params #) .. sctpParams #
#define SOLMEX_SCTP_SCTPABORTED_D "Aborted connections incl. re-transmission drops - (4)"
#define SOLMEX_SCTP_SCTPABORTED_T "counter"
#define SOLMEX_SCTP_SCTPABORTED_N solmex_node_netstat_Sctp_SctpAborted

#define SOLMEX_SCTP_SCTPACTIVEESTAB_D "Active opens - (2)"
#define SOLMEX_SCTP_SCTPACTIVEESTAB_T "counter"
#define SOLMEX_SCTP_SCTPACTIVEESTAB_N solmex_node_netstat_Sctp_SctpActiveEstab

#define SOLMEX_SCTP_SCTPCHECKSUMERROR_D "Packets discarded due to cksum error - (7)"
#define SOLMEX_SCTP_SCTPCHECKSUMERROR_T "counter"
#define SOLMEX_SCTP_SCTPCHECKSUMERROR_N solmex_node_netstat_Sctp_SctpChecksumError

#define SOLMEX_SCTP_SCTPCURRESTAB_D "Connections with state ESTABLISHED, SHUTDOWN-RECEIVED or SHUTDOWN-PENDING - (1)"
#define SOLMEX_SCTP_SCTPCURRESTAB_T "counter"
#define SOLMEX_SCTP_SCTPCURRESTAB_N solmex_node_netstat_Sctp_SctpCurrEstab

#define SOLMEX_SCTP_SCTPFRAGUSRMSGS_D "Fragmented User Messages - (14)"
#define SOLMEX_SCTP_SCTPFRAGUSRMSGS_T "counter"
#define SOLMEX_SCTP_SCTPFRAGUSRMSGS_N solmex_node_netstat_Sctp_SctpFragUsrMsgs

#define SOLMEX_SCTP_SCTPINACK_D "Received SACK chunks"
#define SOLMEX_SCTP_SCTPINACK_T "counter"
#define SOLMEX_SCTP_SCTPINACK_N solmex_node_netstat_Sctp_SctpInAck

#define SOLMEX_SCTP_SCTPINACKUNSENT_D "Received SACK chunks acking unsent data"
#define SOLMEX_SCTP_SCTPINACKUNSENT_T "counter"
#define SOLMEX_SCTP_SCTPINACKUNSENT_N solmex_node_netstat_Sctp_SctpInAckUnsent

#define SOLMEX_SCTP_SCTPINCLOSED_D "Packets received after the association has been closed"
#define SOLMEX_SCTP_SCTPINCLOSED_T "counter"
#define SOLMEX_SCTP_SCTPINCLOSED_N solmex_node_netstat_Sctp_SctpInClosed

#define SOLMEX_SCTP_SCTPINCTRLCHUNKS_D "Control chunks received - (11)"
#define SOLMEX_SCTP_SCTPINCTRLCHUNKS_T "counter"
#define SOLMEX_SCTP_SCTPINCTRLCHUNKS_N solmex_node_netstat_Sctp_SctpInCtrlChunks

#define SOLMEX_SCTP_SCTPINDUPACK_D "Received SACK chunks with duplicate TSN"
#define SOLMEX_SCTP_SCTPINDUPACK_T "counter"
#define SOLMEX_SCTP_SCTPINDUPACK_N solmex_node_netstat_Sctp_SctpInDupAck

#define SOLMEX_SCTP_SCTPININVALIDCOOKIE_D "Invalid cookies received"
#define SOLMEX_SCTP_SCTPININVALIDCOOKIE_T "counter"
#define SOLMEX_SCTP_SCTPININVALIDCOOKIE_N solmex_node_netstat_Sctp_SctpInInvalidCookie

#define SOLMEX_SCTP_SCTPINORDERCHUNKS_D "Ordered data chunks received - (12)"
#define SOLMEX_SCTP_SCTPINORDERCHUNKS_T "counter"
#define SOLMEX_SCTP_SCTPINORDERCHUNKS_N solmex_node_netstat_Sctp_SctpInOrderChunks

#define SOLMEX_SCTP_SCTPINSCTPPKTS_D "Received SCTP incl. OOB packets - (17)"
#define SOLMEX_SCTP_SCTPINSCTPPKTS_T "counter"
#define SOLMEX_SCTP_SCTPINSCTPPKTS_N solmex_node_netstat_Sctp_SctpInSCTPPkts

#define SOLMEX_SCTP_SCTPINUNORDERCHUNKS_D "Unordered data chunks received - (13)"
#define SOLMEX_SCTP_SCTPINUNORDERCHUNKS_T "counter"
#define SOLMEX_SCTP_SCTPINUNORDERCHUNKS_N solmex_node_netstat_Sctp_SctpInUnorderChunks

#define SOLMEX_SCTP_SCTPLISTENDROP_D "Connections refused due to backlog full on listen"
#define SOLMEX_SCTP_SCTPLISTENDROP_T "counter"
#define SOLMEX_SCTP_SCTPLISTENDROP_N solmex_node_netstat_Sctp_SctpListenDrop

#define SOLMEX_SCTP_SCTPMAXASSOCS_D "Max number of associations - (params 5)"
#define SOLMEX_SCTP_SCTPMAXASSOCS_T "counter"
#define SOLMEX_SCTP_SCTPMAXASSOCS_N solmex_node_netstat_Sctp_SctpMaxAssocs

#define SOLMEX_SCTP_SCTPMAXINITRETR_D "Max number of retrans in startup - (params 7)"
#define SOLMEX_SCTP_SCTPMAXINITRETR_T "counter"
#define SOLMEX_SCTP_SCTPMAXINITRETR_N solmex_node_netstat_Sctp_SctpMaxInitRetr

#define SOLMEX_SCTP_SCTPOUTACK_D "SACK chunks sent (incl. delayed chunks)"
#define SOLMEX_SCTP_SCTPOUTACK_T "counter"
#define SOLMEX_SCTP_SCTPOUTACK_N solmex_node_netstat_Sctp_SctpOutAck

#define SOLMEX_SCTP_SCTPOUTACKDELAYED_D "Delayed ACK timeouts"
#define SOLMEX_SCTP_SCTPOUTACKDELAYED_T "counter"
#define SOLMEX_SCTP_SCTPOUTACKDELAYED_N solmex_node_netstat_Sctp_SctpOutAckDelayed

#define SOLMEX_SCTP_SCTPOUTCTRLCHUNKS_D "Control chunks sent - (8)"
#define SOLMEX_SCTP_SCTPOUTCTRLCHUNKS_T "counter"
#define SOLMEX_SCTP_SCTPOUTCTRLCHUNKS_N solmex_node_netstat_Sctp_SctpOutCtrlChunks

#define SOLMEX_SCTP_SCTPOUTFASTRETRANS_D "Segments sent due to 'fast retransmit'"
#define SOLMEX_SCTP_SCTPOUTFASTRETRANS_T "counter"
#define SOLMEX_SCTP_SCTPOUTFASTRETRANS_N solmex_node_netstat_Sctp_SctpOutFastRetrans

#define SOLMEX_SCTP_SCTPOUTOFBLUE_D "OOB packets - (6)"
#define SOLMEX_SCTP_SCTPOUTOFBLUE_T "counter"
#define SOLMEX_SCTP_SCTPOUTOFBLUE_N solmex_node_netstat_Sctp_SctpOutOfBlue

#define SOLMEX_SCTP_SCTPOUTORDERCHUNKS_D "Ordered data chunks sent - (9)"
#define SOLMEX_SCTP_SCTPOUTORDERCHUNKS_T "counter"
#define SOLMEX_SCTP_SCTPOUTORDERCHUNKS_N solmex_node_netstat_Sctp_SctpOutOrderChunks

#define SOLMEX_SCTP_SCTPOUTSCTPPKTS_D "Sent SCTP Packets - (16)"
#define SOLMEX_SCTP_SCTPOUTSCTPPKTS_T "counter"
#define SOLMEX_SCTP_SCTPOUTSCTPPKTS_N solmex_node_netstat_Sctp_SctpOutSCTPPkts

#define SOLMEX_SCTP_SCTPOUTUNORDERCHUNKS_D "Unordered data chunks sent - (10)"
#define SOLMEX_SCTP_SCTPOUTUNORDERCHUNKS_T "counter"
#define SOLMEX_SCTP_SCTPOUTUNORDERCHUNKS_N solmex_node_netstat_Sctp_SctpOutUnorderChunks

#define SOLMEX_SCTP_SCTPOUTWINPROBE_D "Window probes sent"
#define SOLMEX_SCTP_SCTPOUTWINPROBE_T "counter"
#define SOLMEX_SCTP_SCTPOUTWINPROBE_N solmex_node_netstat_Sctp_SctpOutWinProbe

#define SOLMEX_SCTP_SCTPOUTWINUPDATE_D "SACK chunks sent to update window"
#define SOLMEX_SCTP_SCTPOUTWINUPDATE_T "counter"
#define SOLMEX_SCTP_SCTPOUTWINUPDATE_N solmex_node_netstat_Sctp_SctpOutWinUpdate

#define SOLMEX_SCTP_SCTPPASSIVEESTAB_D "Passive opens - (3)"
#define SOLMEX_SCTP_SCTPPASSIVEESTAB_T "counter"
#define SOLMEX_SCTP_SCTPPASSIVEESTAB_N solmex_node_netstat_Sctp_SctpPassiveEstab

#define SOLMEX_SCTP_SCTPREASMUSRMSGS_D "Reassembled User Messages - (15)"
#define SOLMEX_SCTP_SCTPREASMUSRMSGS_T "counter"
#define SOLMEX_SCTP_SCTPREASMUSRMSGS_N solmex_node_netstat_Sctp_SctpReasmUsrMsgs

#define SOLMEX_SCTP_SCTPRETRANSCHUNKS_D "Retransmitted data chunks"
#define SOLMEX_SCTP_SCTPRETRANSCHUNKS_T "counter"
#define SOLMEX_SCTP_SCTPRETRANSCHUNKS_N solmex_node_netstat_Sctp_SctpRetransChunks

#define SOLMEX_SCTP_SCTPRTOALGORITHM_D "Algorithm used to determine retransmission timeout (1 .. other, 2 .. vanj) - (params 1)"
#define SOLMEX_SCTP_SCTPRTOALGORITHM_T "counter"
#define SOLMEX_SCTP_SCTPRTOALGORITHM_N solmex_node_netstat_Sctp_SctpRtoAlgorithm

#define SOLMEX_SCTP_SCTPRTOINITIAL_D "Initial retransmission timeout in ms - (params 4)"
#define SOLMEX_SCTP_SCTPRTOINITIAL_T "counter"
#define SOLMEX_SCTP_SCTPRTOINITIAL_N solmex_node_netstat_Sctp_SctpRtoInitial

#define SOLMEX_SCTP_SCTPRTOMAX_D "Max retransmission timeout in ms - (params 3)"
#define SOLMEX_SCTP_SCTPRTOMAX_T "counter"
#define SOLMEX_SCTP_SCTPRTOMAX_N solmex_node_netstat_Sctp_SctpRtoMax

#define SOLMEX_SCTP_SCTPRTOMIN_D "Min retransmission timeout in ms - (params 2)"
#define SOLMEX_SCTP_SCTPRTOMIN_T "counter"
#define SOLMEX_SCTP_SCTPRTOMIN_N solmex_node_netstat_Sctp_SctpRtoMin

#define SOLMEX_SCTP_SCTPSHUTDOWNS_D "Graceful shutdowns - (5)"
#define SOLMEX_SCTP_SCTPSHUTDOWNS_T "counter"
#define SOLMEX_SCTP_SCTPSHUTDOWNS_N solmex_node_netstat_Sctp_SctpShutdowns

#define SOLMEX_SCTP_SCTPTIMHEARBEATDROP_D "Heartbeat probes"
#define SOLMEX_SCTP_SCTPTIMHEARBEATDROP_T "counter"
#define SOLMEX_SCTP_SCTPTIMHEARBEATDROP_N solmex_node_netstat_Sctp_SctpTimHearBeatDrop

#define SOLMEX_SCTP_SCTPTIMHEARBEATPROBE_D "Heartbeat timeouts dropping the connection"
#define SOLMEX_SCTP_SCTPTIMHEARBEATPROBE_T "counter"
#define SOLMEX_SCTP_SCTPTIMHEARBEATPROBE_N solmex_node_netstat_Sctp_SctpTimHearBeatProbe

#define SOLMEX_SCTP_SCTPTIMRETRANS_D "Retransmit timeouts"
#define SOLMEX_SCTP_SCTPTIMRETRANS_T "counter"
#define SOLMEX_SCTP_SCTPTIMRETRANS_N solmex_node_netstat_Sctp_SctpTimRetrans

#define SOLMEX_SCTP_SCTPTIMRETRANSDROP_D "Retransmit timeouts dropping the connection"
#define SOLMEX_SCTP_SCTPTIMRETRANSDROP_T "counter"
#define SOLMEX_SCTP_SCTPTIMRETRANSDROP_N solmex_node_netstat_Sctp_SctpTimRetransDrop

#define SOLMEX_SCTP_SCTPVALCOOKIELIFE_D "Cookie lifetime in ms - (params 6)"
#define SOLMEX_SCTP_SCTPVALCOOKIELIFE_T "counter"
#define SOLMEX_SCTP_SCTPVALCOOKIELIFE_N solmex_node_netstat_Sctp_SctpValCookieLife

// (#) .. tcp #
#define SOLMEX_TCP_ACTIVEOPENS_D "Direct transitions CLOSED -> SYN-SENT - (5)"
#define SOLMEX_TCP_ACTIVEOPENS_T "counter"
#define SOLMEX_TCP_ACTIVEOPENS_N solmex_node_netstat_Tcp_ActiveOpens

#define SOLMEX_TCP_ATTEMPTFAILS_D "Direct transition SIN-SENT/RCVD -> CLOSED/LISTEN - (7)"
#define SOLMEX_TCP_ATTEMPTFAILS_T "counter"
#define SOLMEX_TCP_ATTEMPTFAILS_N solmex_node_netstat_Tcp_AttemptFails

#define SOLMEX_TCP_CONNTABLESIZE_D "Size of a IPv4 connection table entry (tcpConnEntry_t) in bytes - (13)"
#define SOLMEX_TCP_CONNTABLESIZE_T "counter"
#define SOLMEX_TCP_CONNTABLESIZE_N solmex_node_netstat_Tcp_ConnTableSize

#define SOLMEX_TCP_CONNTABLESIZE6_D "Size of a IPv6 connection table entry (tcp6ConnEntry_t) in bytes"
#define SOLMEX_TCP_CONNTABLESIZE6_T "counter"
#define SOLMEX_TCP_CONNTABLESIZE6_N solmex_node_netstat_Tcp_ConnTableSize6

#define SOLMEX_TCP_CURRESTAB_D "Connections in ESTABLISHED or CLOSE-WAIT state - (9)"
#define SOLMEX_TCP_CURRESTAB_T "counter"
#define SOLMEX_TCP_CURRESTAB_N solmex_node_netstat_Tcp_CurrEstab

#define SOLMEX_TCP_ESTABRESETS_D "Direct transitions ESTABLISHED|CLOSE-WAIT -> CLOSED - (8)"
#define SOLMEX_TCP_ESTABRESETS_T "counter"
#define SOLMEX_TCP_ESTABRESETS_N solmex_node_netstat_Tcp_EstabResets

#define SOLMEX_TCP_HALFOPENDROP_D "Connections dropped from a full half-open queue (q0)"
#define SOLMEX_TCP_HALFOPENDROP_T "counter"
#define SOLMEX_TCP_HALFOPENDROP_N solmex_node_netstat_Tcp_HalfOpenDrop

#define SOLMEX_TCP_INACKBYTES_D "Number of bytes ACKed"
#define SOLMEX_TCP_INACKBYTES_T "counter"
#define SOLMEX_TCP_INACKBYTES_N solmex_node_netstat_Tcp_InAckBytes

#define SOLMEX_TCP_INACKSEGS_D "Number of ACK segments received"
#define SOLMEX_TCP_INACKSEGS_T "counter"
#define SOLMEX_TCP_INACKSEGS_N solmex_node_netstat_Tcp_InAckSegs

#define SOLMEX_TCP_INACKUNSENT_D "Number of ACKs acking unsent data"
#define SOLMEX_TCP_INACKUNSENT_T "counter"
#define SOLMEX_TCP_INACKUNSENT_N solmex_node_netstat_Tcp_InAckUnsent

#define SOLMEX_TCP_INCLOSED_D "Data segments received after the connection has closed"
#define SOLMEX_TCP_INCLOSED_T "counter"
#define SOLMEX_TCP_INCLOSED_N solmex_node_netstat_Tcp_InClosed

#define SOLMEX_TCP_INDATADUPBYTES_D "Complete duplicate data segments received in bytes"
#define SOLMEX_TCP_INDATADUPBYTES_T "counter"
#define SOLMEX_TCP_INDATADUPBYTES_N solmex_node_netstat_Tcp_InDataDupBytes

#define SOLMEX_TCP_INDATADUPSEGS_D "Complete duplicate data segments received"
#define SOLMEX_TCP_INDATADUPSEGS_T "counter"
#define SOLMEX_TCP_INDATADUPSEGS_N solmex_node_netstat_Tcp_InDataDupSegs

#define SOLMEX_TCP_INDATAINORDERBYTES_D "In order received data segments in bytes"
#define SOLMEX_TCP_INDATAINORDERBYTES_T "counter"
#define SOLMEX_TCP_INDATAINORDERBYTES_N solmex_node_netstat_Tcp_InDataInorderBytes

#define SOLMEX_TCP_INDATAINORDERSEGS_D "In order received data segments"
#define SOLMEX_TCP_INDATAINORDERSEGS_T "counter"
#define SOLMEX_TCP_INDATAINORDERSEGS_N solmex_node_netstat_Tcp_InDataInorderSegs

#define SOLMEX_TCP_INDATAPARTDUPBYTES_D "Partial duplicate data segments received in bytes"
#define SOLMEX_TCP_INDATAPARTDUPBYTES_T "counter"
#define SOLMEX_TCP_INDATAPARTDUPBYTES_N solmex_node_netstat_Tcp_InDataPartDupBytes

#define SOLMEX_TCP_INDATAPARTDUPSEGS_D "Partial duplicate data segments received"
#define SOLMEX_TCP_INDATAPARTDUPSEGS_T "counter"
#define SOLMEX_TCP_INDATAPARTDUPSEGS_N solmex_node_netstat_Tcp_InDataPartDupSegs

#define SOLMEX_TCP_INDATAPASTWINBYTES_D "Data segments received past the window in bytes"
#define SOLMEX_TCP_INDATAPASTWINBYTES_T "counter"
#define SOLMEX_TCP_INDATAPASTWINBYTES_N solmex_node_netstat_Tcp_InDataPastWinBytes

#define SOLMEX_TCP_INDATAPASTWINSEGS_D "Data segments received past the window"
#define SOLMEX_TCP_INDATAPASTWINSEGS_T "counter"
#define SOLMEX_TCP_INDATAPASTWINSEGS_N solmex_node_netstat_Tcp_InDataPastWinSegs

#define SOLMEX_TCP_INDATAUNORDERBYTES_D "Data segments received out of order in bytes"
#define SOLMEX_TCP_INDATAUNORDERBYTES_T "counter"
#define SOLMEX_TCP_INDATAUNORDERBYTES_N solmex_node_netstat_Tcp_InDataUnorderBytes

#define SOLMEX_TCP_INDATAUNORDERSEGS_D "Data segments received out of order"
#define SOLMEX_TCP_INDATAUNORDERSEGS_T "counter"
#define SOLMEX_TCP_INDATAUNORDERSEGS_N solmex_node_netstat_Tcp_InDataUnorderSegs

#define SOLMEX_TCP_INDUPACK_D "Duplicate ACKs received"
#define SOLMEX_TCP_INDUPACK_T "counter"
#define SOLMEX_TCP_INDUPACK_N solmex_node_netstat_Tcp_InDupAck

#define SOLMEX_TCP_INSEGS_D "Segments received - (17:10)"
#define SOLMEX_TCP_INSEGS_T "counter"
#define SOLMEX_TCP_INSEGS_N solmex_node_netstat_Tcp_InSegs

#define SOLMEX_TCP_INWINPROBE_D "Zero window probes received"
#define SOLMEX_TCP_INWINPROBE_T "counter"
#define SOLMEX_TCP_INWINPROBE_N solmex_node_netstat_Tcp_InWinProbe

#define SOLMEX_TCP_INWINUPDATE_D "Window updates received"
#define SOLMEX_TCP_INWINUPDATE_T "counter"
#define SOLMEX_TCP_INWINUPDATE_N solmex_node_netstat_Tcp_InWinUpdate

#define SOLMEX_TCP_LISTENDROP_D "Connections refused due to backlog full on listen"
#define SOLMEX_TCP_LISTENDROP_T "counter"
#define SOLMEX_TCP_LISTENDROP_N solmex_node_netstat_Tcp_ListenDrop

#define SOLMEX_TCP_LISTENDROPQ0_D "Connections refused due to half-open queue (q0) full"
#define SOLMEX_TCP_LISTENDROPQ0_T "counter"
#define SOLMEX_TCP_LISTENDROPQ0_N solmex_node_netstat_Tcp_ListenDropQ0

#define SOLMEX_TCP_MAXCONN_D "Maximum number of connections supported - (4)"
#define SOLMEX_TCP_MAXCONN_T "counter"
#define SOLMEX_TCP_MAXCONN_N solmex_node_netstat_Tcp_MaxConn

#define SOLMEX_TCP_OUTACK_D "ACKs sent"
#define SOLMEX_TCP_OUTACK_T "counter"
#define SOLMEX_TCP_OUTACK_N solmex_node_netstat_Tcp_OutAck

#define SOLMEX_TCP_OUTACKDELAYED_D "Delayed ACKs sent"
#define SOLMEX_TCP_OUTACKDELAYED_T "counter"
#define SOLMEX_TCP_OUTACKDELAYED_N solmex_node_netstat_Tcp_OutAckDelayed

#define SOLMEX_TCP_OUTCONTROL_D "Control segments sent (syn, fin, rst)"
#define SOLMEX_TCP_OUTCONTROL_T "counter"
#define SOLMEX_TCP_OUTCONTROL_N solmex_node_netstat_Tcp_OutControl

#define SOLMEX_TCP_OUTDATABYTES_D "Data segments sent in bytes"
#define SOLMEX_TCP_OUTDATABYTES_T "counter"
#define SOLMEX_TCP_OUTDATABYTES_N solmex_node_netstat_Tcp_OutDataBytes

#define SOLMEX_TCP_OUTDATASEGS_D "Data segments sent"
#define SOLMEX_TCP_OUTDATASEGS_T "counter"
#define SOLMEX_TCP_OUTDATASEGS_N solmex_node_netstat_Tcp_OutDataSegs

#define SOLMEX_TCP_OUTFASTRETRANS_D "Segments sent due to 'fast retransmit'"
#define SOLMEX_TCP_OUTFASTRETRANS_T "counter"
#define SOLMEX_TCP_OUTFASTRETRANS_N solmex_node_netstat_Tcp_OutFastRetrans

#define SOLMEX_TCP_OUTRSTS_D "Segments sent with RST flag - (15)"
#define SOLMEX_TCP_OUTRSTS_T "counter"
#define SOLMEX_TCP_OUTRSTS_N solmex_node_netstat_Tcp_OutRsts

#define SOLMEX_TCP_OUTSACKRETRANSSEGS_D "Segments retransmitted by SACK retransmission"
#define SOLMEX_TCP_OUTSACKRETRANSSEGS_T "counter"
#define SOLMEX_TCP_OUTSACKRETRANSSEGS_N solmex_node_netstat_Tcp_OutSackRetransSegs

#define SOLMEX_TCP_OUTSEGS_D "Segments sent - (18:11)"
#define SOLMEX_TCP_OUTSEGS_T "counter"
#define SOLMEX_TCP_OUTSEGS_N solmex_node_netstat_Tcp_OutSegs

#define SOLMEX_TCP_OUTURG_D "Segments sent with the URG flag"
#define SOLMEX_TCP_OUTURG_T "counter"
#define SOLMEX_TCP_OUTURG_N solmex_node_netstat_Tcp_OutUrg

#define SOLMEX_TCP_OUTWINPROBE_D "Zero window probes sent"
#define SOLMEX_TCP_OUTWINPROBE_T "counter"
#define SOLMEX_TCP_OUTWINPROBE_N solmex_node_netstat_Tcp_OutWinProbe

#define SOLMEX_TCP_OUTWINUPDATE_D "Window updates sent"
#define SOLMEX_TCP_OUTWINUPDATE_T "counter"
#define SOLMEX_TCP_OUTWINUPDATE_N solmex_node_netstat_Tcp_OutWinUpdate

#define SOLMEX_TCP_PASSIVEOPENS_D "Direct transitions LISTEN -> SYN-RCVD - (6)"
#define SOLMEX_TCP_PASSIVEOPENS_T "counter"
#define SOLMEX_TCP_PASSIVEOPENS_N solmex_node_netstat_Tcp_PassiveOpens

#define SOLMEX_TCP_RETRANSBYTES_D "Segments retransmitted in bytes"
#define SOLMEX_TCP_RETRANSBYTES_T "counter"
#define SOLMEX_TCP_RETRANSBYTES_N solmex_node_netstat_Tcp_RetransBytes

#define SOLMEX_TCP_RETRANSSEGS_D "Segments retransmitted - (12)"
#define SOLMEX_TCP_RETRANSSEGS_T "counter"
#define SOLMEX_TCP_RETRANSSEGS_N solmex_node_netstat_Tcp_RetransSegs

#define SOLMEX_TCP_RTOALGORITHM_D "Algorithm used for retransmission timeout value - (1)"
#define SOLMEX_TCP_RTOALGORITHM_T "gauge"
#define SOLMEX_TCP_RTOALGORITHM_N solmex_node_netstat_Tcp_RtoAlgorithm

#define SOLMEX_TCP_RTOMAX_D "Maximum retransmission timeout in ms - (3)"
#define SOLMEX_TCP_RTOMAX_T "gauge"
#define SOLMEX_TCP_RTOMAX_N solmex_node_netstat_Tcp_RtoMax

#define SOLMEX_TCP_RTOMIN_D "Minimum retransmission timeout in ms - (2)"
#define SOLMEX_TCP_RTOMIN_T "gauge"
#define SOLMEX_TCP_RTOMIN_N solmex_node_netstat_Tcp_RtoMin

#define SOLMEX_TCP_RTTNOUPDATE_D "Failed attempts to update the rtt estimate"
#define SOLMEX_TCP_RTTNOUPDATE_T "counter"
#define SOLMEX_TCP_RTTNOUPDATE_N solmex_node_netstat_Tcp_RttNoUpdate

#define SOLMEX_TCP_RTTUPDATE_D "Successful attempts to update the rtt estimate"
#define SOLMEX_TCP_RTTUPDATE_T "counter"
#define SOLMEX_TCP_RTTUPDATE_N solmex_node_netstat_Tcp_RttUpdate

#define SOLMEX_TCP_TIMKEEPALIVE_D "Keepalive timeouts"
#define SOLMEX_TCP_TIMKEEPALIVE_T "counter"
#define SOLMEX_TCP_TIMKEEPALIVE_N solmex_node_netstat_Tcp_TimKeepalive

#define SOLMEX_TCP_TIMKEEPALIVEDROP_D "Keepalive timeouts dropping the connection"
#define SOLMEX_TCP_TIMKEEPALIVEDROP_T "counter"
#define SOLMEX_TCP_TIMKEEPALIVEDROP_N solmex_node_netstat_Tcp_TimKeepaliveDrop

#define SOLMEX_TCP_TIMKEEPALIVEPROBE_D "Keepalive timeouts sending a probe"
#define SOLMEX_TCP_TIMKEEPALIVEPROBE_T "counter"
#define SOLMEX_TCP_TIMKEEPALIVEPROBE_N solmex_node_netstat_Tcp_TimKeepaliveProbe

#define SOLMEX_TCP_TIMRETRANS_D "Retransmit timeouts"
#define SOLMEX_TCP_TIMRETRANS_T "counter"
#define SOLMEX_TCP_TIMRETRANS_N solmex_node_netstat_Tcp_TimRetrans

#define SOLMEX_TCP_TIMRETRANSDROP_D "Retransmit timeouts dropping the connection"
#define SOLMEX_TCP_TIMRETRANSDROP_T "counter"
#define SOLMEX_TCP_TIMRETRANSDROP_N solmex_node_netstat_Tcp_TimRetransDrop


#define SOLMEX_UDP_ENTRY6SIZE_D ""
#define SOLMEX_UDP_ENTRY6SIZE_T "counter"
#define SOLMEX_UDP_ENTRY6SIZE_N solmex_node_netstat_Udp_Entry6Size

#define SOLMEX_UDP_ENTRYSIZE_D ""
#define SOLMEX_UDP_ENTRYSIZE_T "counter"
#define SOLMEX_UDP_ENTRYSIZE_N solmex_node_netstat_Udp_EntrySize

#define SOLMEX_UDP_INDATAGRAMS_D ""
#define SOLMEX_UDP_INDATAGRAMS_T "counter"
#define SOLMEX_UDP_INDATAGRAMS_N solmex_node_netstat_Udp_InDatagrams

#define SOLMEX_UDP_INERRORS_D ""
#define SOLMEX_UDP_INERRORS_T "counter"
#define SOLMEX_UDP_INERRORS_N solmex_node_netstat_Udp_InErrors

#define SOLMEX_UDP_OUTDATAGRAMS_D ""
#define SOLMEX_UDP_OUTDATAGRAMS_T "counter"
#define SOLMEX_UDP_OUTDATAGRAMS_N solmex_node_netstat_Udp_OutDatagrams

#define SOLMEX_UDP_OUTERRORS_D ""
#define SOLMEX_UDP_OUTERRORS_T "counter"
#define SOLMEX_UDP_OUTERRORS_N solmex_node_netstat_Udp_OutErrors

/*
#define SOLMEXM_XXX_D "short description."
#define SOLMEXM_XXX_T "gauge"
#define SOLMEXM_XXX_N "solmex_yyy"
 */

#ifdef __cplusplus
}
#endif

#endif // SOLMEX_COMMON_H
