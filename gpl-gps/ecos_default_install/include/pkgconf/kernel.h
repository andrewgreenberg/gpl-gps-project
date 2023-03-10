#ifndef CYGONCE_PKGCONF_KERNEL_H
#define CYGONCE_PKGCONF_KERNEL_H
/*
 * File <pkgconf/kernel.h>
 *
 * This file is generated automatically by the configuration
 * system. It should not be edited. Any changes to this file
 * may be overwritten.
 */

/***** proc output start *****/
#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/infra.h>
#include CYGBLD_HAL_PLATFORM_H
#ifndef CYGNUM_HAL_RTC_NUMERATOR
# define CYGNUM_HAL_RTC_NUMERATOR     1000000000
# define CYGNUM_HAL_RTC_DENOMINATOR   100
# define CYGNUM_HAL_RTC_PERIOD        9999
#endif
/*****  proc output end  *****/
#define CYGPKG_KERNEL_INTERRUPTS 1
#define CYGIMP_KERNEL_INTERRUPTS_DSRS 1
#define CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST 1
#define CYGSEM_KERNEL_INTERRUPTS_DSRS_LIST_FIFO 1
#define CYGPKG_KERNEL_EXCEPTIONS 1
#define CYGSEM_KERNEL_EXCEPTIONS_GLOBAL 1
#define CYGPKG_KERNEL_SCHED 1
#define CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES 0
#define CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES_0
#define CYGSEM_KERNEL_SCHED_MLQUEUE 1
#define CYGPRI_KERNEL_SCHED_IMPL_HXX <cyg/kernel/mlqueue.hxx>
#define CYGNUM_KERNEL_SCHED_PRIORITIES 32
#define CYGNUM_KERNEL_SCHED_PRIORITIES_32
#define CYGNUM_KERNEL_SCHED_BITMAP_SIZE CYGNUM_KERNEL_SCHED_PRIORITIES
#define CYGNUM_KERNEL_SCHED_BITMAP_SIZE_CYGNUM_KERNEL_SCHED_PRIORITIES
#define CYGSEM_KERNEL_SCHED_TIMESLICE 1
#define CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS 5
#define CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS_5
#define CYGPKG_KERNEL_COUNTERS 1
#define CYGVAR_KERNEL_COUNTERS_CLOCK 1
#define CYGNUM_KERNEL_COUNTERS_CLOCK_ISR_PRIORITY 1
#define CYGNUM_KERNEL_COUNTERS_CLOCK_ISR_PRIORITY_1
#define CYGIMP_KERNEL_COUNTERS_SINGLE_LIST 1
#define CYGNUM_KERNEL_COUNTERS_RTC_RESOLUTION {CYGNUM_HAL_RTC_NUMERATOR, CYGNUM_HAL_RTC_DENOMINATOR}
#define CYGNUM_KERNEL_COUNTERS_RTC_PERIOD CYGNUM_HAL_RTC_PERIOD
#define CYGNUM_KERNEL_COUNTERS_RTC_PERIOD_CYGNUM_HAL_RTC_PERIOD
#define CYGPKG_KERNEL_THREADS 1
#define CYGFUN_KERNEL_THREADS_TIMER 1
#define CYGVAR_KERNEL_THREADS_NAME 1
#define CYGVAR_KERNEL_THREADS_LIST 1
#define CYGFUN_KERNEL_THREADS_STACK_LIMIT 1
#define CYGFUN_KERNEL_THREADS_STACK_CHECKING 1
#define CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE 32
#define CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE_32
#define CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT 1
#define CYGVAR_KERNEL_THREADS_DATA 1
#define CYGNUM_KERNEL_THREADS_DATA_MAX 6
#define CYGNUM_KERNEL_THREADS_DATA_MAX_6
#define CYGNUM_KERNEL_THREADS_DATA_ALL 15
#define CYGNUM_KERNEL_THREADS_DATA_ALL_15
#define CYGNUM_KERNEL_THREADS_DATA_KERNEL 0
#define CYGNUM_KERNEL_THREADS_DATA_KERNEL_0
#define CYGNUM_KERNEL_THREADS_DATA_ITRON 1
#define CYGNUM_KERNEL_THREADS_DATA_ITRON_1
#define CYGNUM_KERNEL_THREADS_DATA_ERRNO 2
#define CYGNUM_KERNEL_THREADS_DATA_ERRNO_2
#define CYGNUM_KERNEL_THREADS_DATA_POSIX 3
#define CYGNUM_KERNEL_THREADS_DATA_POSIX_3
#define CYGNUM_KERNEL_THREADS_IDLE_STACK_SIZE 2048
#define CYGNUM_KERNEL_THREADS_IDLE_STACK_SIZE_2048
#define CYGNUM_KERNEL_MAX_SUSPEND_COUNT_ASSERT 500
#define CYGNUM_KERNEL_MAX_SUSPEND_COUNT_ASSERT_500
#define CYGNUM_KERNEL_MAX_COUNTED_WAKE_COUNT_ASSERT 500
#define CYGNUM_KERNEL_MAX_COUNTED_WAKE_COUNT_ASSERT_500
#define CYGPKG_KERNEL_SYNCH 1
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL SIMPLE
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_SIMPLE
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT 1
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING 1
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY 0
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY_0
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_NONE 1
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT INHERIT
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_INHERIT
#define CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC 1
#define CYGINT_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_COUNT 3
#define CYGINT_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_COUNT_3
#define CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT 1
#define CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE 10
#define CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE_10
#define CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT 1
#define CYGMFN_KERNEL_SYNCH_CONDVAR_WAIT_MUTEX 1
#define CYGPKG_KERNEL_DEBUG 1
#define CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT 1
#define CYGPKG_KERNEL_API 1
#define CYGFUN_KERNEL_API_C 1
#define CYGPKG_KERNEL_OPTIONS 1

#endif
