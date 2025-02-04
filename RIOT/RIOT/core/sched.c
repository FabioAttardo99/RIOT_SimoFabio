/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     core_sched
 * @{
 *
 * @file
 * @brief       Scheduler implementation
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      René Kijewski <rene.kijewski@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>

#include "sched.h"
#include "clist.h"
#include "bitarithm.h"
#include "irq.h"
#include "thread.h"
#include "log.h"
#include "xtimer.h"

#ifdef MODULE_MPU_STACK_GUARD
#include "mpu.h"
#endif

#define ENABLE_DEBUG (1)
#include "debug.h"

#if ENABLE_DEBUG
/* For PRIu16 etc. */
#include <inttypes.h>
#endif

#ifdef PICOLIBC_TLS
#include <picotls.h>
#endif

/* Needed by OpenOCD to read sched_threads */
#if defined(__APPLE__) && defined(__MACH__)
 #define FORCE_USED_SECTION __attribute__((used)) __attribute__((section( \
                                                                     "__OPENOCD,__openocd")))
#else
 #define FORCE_USED_SECTION __attribute__((used)) __attribute__((section( \
                                                                     ".openocd")))
#endif

/**
 * @brief   Symbols also used by OpenOCD, keep in sync with src/rtos/riot.c
 * @{
 */
volatile kernel_pid_t sched_active_pid = KERNEL_PID_UNDEF;
volatile thread_t *sched_threads[KERNEL_PID_LAST + 1];
volatile int sched_num_threads = 0;

FORCE_USED_SECTION
const uint8_t max_threads = ARRAY_SIZE(sched_threads);

#ifdef DEVELHELP
/* OpenOCD can't determine struct offsets and additionally this member is only
 * available if compiled with DEVELHELP */
FORCE_USED_SECTION
const uint8_t _tcb_name_offset = offsetof(thread_t, name);
#endif
/** @} */

volatile thread_t *sched_active_thread;
volatile unsigned int sched_context_switch_request;

clist_node_t sched_runqueues[SCHED_PRIO_LEVELS];
static uint32_t runqueue_bitcache = 0;

#ifdef MODULE_SCHED_CB
static void (*sched_cb) (kernel_pid_t active_thread,
                         kernel_pid_t next_thread) = NULL;
#endif

/* Depending on whether the CLZ instruction is available, the order of the
 * runqueue_bitcache is reversed. When the instruction is available, it is
 * faster to determine the MSBit set. When it is not available it is faster to
 * determine the LSBit set. These functions abstract the runqueue modifications
 * and readout away, switching between the two orders depending on the CLZ
 * instruction availability
 */
static inline void _set_runqueue_bit(thread_t *process)
{
#if defined(BITARITHM_HAS_CLZ)
    runqueue_bitcache |= BIT31 >> process->priority;
#else
    runqueue_bitcache |= 1 << process->priority;
#endif
}

static inline void _clear_runqueue_bit(thread_t *process)
{
#if defined(BITARITHM_HAS_CLZ)
    runqueue_bitcache &= ~(BIT31 >> process->priority);
#else
    runqueue_bitcache &= ~(1 << process->priority);
#endif
}

static inline unsigned _get_prio_queue_from_runqueue(void)
{
#if defined(BITARITHM_HAS_CLZ)
    return 31 - bitarithm_msb(runqueue_bitcache);
#else
    return bitarithm_lsb(runqueue_bitcache);
#endif
}

/*
static int _print_node(clist_node_t *node)
    {
         printf("0x%08x ", (unsigned)node);
         return 0;
    }
*/

static void _unschedule(thread_t *active_thread)
{
    if (active_thread->status == STATUS_RUNNING) {
        active_thread->status = STATUS_PENDING;
    }


#ifdef SCHED_TEST_STACK
    if (*((uintptr_t *)active_thread->stack_start) !=
        (uintptr_t)active_thread->stack_start) {
        LOG_WARNING(
            "scheduler(): stack overflow detected, pid=%" PRIkernel_pid "\n",
            active_thread->pid);
    }
#endif
#ifdef MODULE_SCHED_CB
    if (sched_cb) {
        sched_cb(active_thread->pid, KERNEL_PID_UNDEF);
    }
#endif
}
/*
typedef void (*xtimer_callback_t)(void*);
xtimer_t Run = {
		NULL,
		0,
		0,
		0,
		0,
		(xtimer_callback_t) sched_run,
		NULL
	};
uint32_t Time = 500;
*/


// Chiama la funzione sched_run
static void callSchedRun(void *arg) {
    (void) arg;
    DEBUG("Sto chiamando sched_run\n");
    sched_run();
}

static xtimer_t Run = {
    .next = NULL,
    .offset = 0,
    .long_offset = 0,
    .start_time = 0,
    .long_start_time = 0,
    .callback = callSchedRun,
    .arg = NULL
    }
    ;

void *__attribute__((used)) sched_run(void)
{   
    
/*
    for (int i = 0; i < SCHED_PRIO_LEVELS; i++) {
        if ((int)clist_count(&sched_runqueues[i]) != 0)
        DEBUG(" -- SCHED_RUN -- CODA %d -> %d \n", i, (int)clist_count(&sched_runqueues[i]));
    }
  
    for (int i = 0; i < SCHED_PRIO_LEVELS; i++) 
    {

	DEBUG(" -- SCHED_RUN -- CODA %d -> %d \n", i, (int)clist_count(&sched_runqueues[i]));
	if((int)clist_count(&sched_runqueues[i]) != 0)
	{
		//clist_node_t *list = clist_lpeek(&sched_runqueues[i]);
		
		for(int j = 0; j < (int)clist_count(&sched_runqueues[i]);++j)
		{
			//clist_node_t *node = list->next;
			 thread_t *t = container_of(sched_runqueues[i].next->next,
                                         thread_t, rq_entry);
			DEBUG(" -- List --Pos: %d -> %d name: %s\n", j, t->pid,t->name);
			clist_lpoprpush(&sched_runqueues[i]); 
		}
	}
    }	
    DEBUG("Runqueue: %d, Funzione: %d \n", runqueue_bitcache,_get_prio_queue_from_runqueue());

*/
    
    thread_t *active_thread = thread_get_active();
    thread_t *previous_thread = active_thread;
     

    if (!IS_USED(MODULE_CORE_IDLE_THREAD) && !runqueue_bitcache) {
        if (active_thread) {
            _unschedule(active_thread);
            active_thread = NULL;
        }

        do {
            DEBUG("Non ho trovato altri thread!");
            sched_arch_idle();
        } while (!runqueue_bitcache);
    }

    sched_context_switch_request = 0;

    unsigned nextrq = _get_prio_queue_from_runqueue();
    thread_t *next_thread = container_of(sched_runqueues[nextrq].next->next,
                                         thread_t, rq_entry);

    //  Se pid main o idle, no decremento servicetime
    if (active_thread != NULL) {
        if((int)next_thread -> pid != 1 && (int)next_thread -> pid != 2) {
            DEBUG("\n Pid: %d, S_TIME: %d \n",(int)next_thread->pid, next_thread -> s_time);
            if (next_thread -> s_time >= 500) next_thread -> s_time -= 500;
            else next_thread -> s_time = 0;
            DEBUG("\n Pid: %d, S_TIME_AFTER: %d \n",(int)next_thread->pid, next_thread -> s_time);
    }
}      

    DEBUG( "sched_run: active thread: %" PRIkernel_pid ", next thread: %" PRIkernel_pid "\n",
        (kernel_pid_t)((active_thread == NULL)
                       ? KERNEL_PID_UNDEF
                       : active_thread->pid),
        next_thread->pid);

    next_thread->status = STATUS_RUNNING;

    if (previous_thread == next_thread) {
#ifdef MODULE_SCHED_CB
        /* Call the sched callback again only if the active thread is NULL. When
         * active_thread is NULL, there was a sleep in between descheduling the
         * previous thread and scheduling the new thread. Call the callback here
         * again ensures that the time sleeping doesn't count as running the
         * previous thread
         */
        if (sched_cb && !active_thread) {
            sched_cb(KERNEL_PID_UNDEF, next_thread->pid);
        }
#endif
        DEBUG("sched_run: done, sched_active_thread was not changed.\n");
    }
    else {
        if (active_thread) {
            _unschedule(active_thread);
        }

        sched_active_pid = next_thread->pid;
        sched_active_thread = next_thread;

#ifdef MODULE_SCHED_CB
        if (sched_cb) {
            sched_cb(KERNEL_PID_UNDEF, next_thread->pid);
        }
#endif

#ifdef PICOLIBC_TLS
        _set_tls(next_thread->tls);
#endif

#ifdef MODULE_MPU_STACK_GUARD
        mpu_configure(
            2,                                              /* MPU region 2 */
            (uintptr_t)next_thread->stack_start + 31,       /* Base Address (rounded up) */
            MPU_ATTR(1, AP_RO_RO, 0, 1, 0, 1, MPU_SIZE_32B) /* Attributes and Size */
            );
#endif
        DEBUG("NEXT_THREAD: %d, SERVICE_TIME: %d\n", (int)next_thread->pid, next_thread->s_time);
        int n = (int)next_thread->pid;
        if (next_thread->s_time == 0) {
            if (n != 1 && n != 2) {
                DEBUG("sched_run: Actual task: %d, SERVICE TIME ENDED! Exiting from current task! \n", (int)next_thread->pid);
                sched_task_exit();
            }
        }
        DEBUG("sched_run: done, changed sched_active_thread.\n");
    }
    
 //   if ((int)next_thread->pid != 2) {
    xtimer_set(&Run, 500000LU);
    DEBUG("*** TIMER SETTED SUCCESSFULLY *** \n");
 //   }                
/*
bool Check = true;
uint32_t Time = xtimer_now_usec(); 
while(Check)
{
    Time = xtimer_now_usec(); 
    if(Time >= 1000000)
    {
        printf("TIME: %d \n" , Time);
        Check = false;
    }
}
*/
    clist_lpoprpush(&sched_runqueues[1]);
    DEBUG("CHIUDO SCHED_RUN\n");
    return NULL;
 //   return next_thread;
}

void checkTask(kernel_pid_t pid) {
    if (thread_get(pid)->s_time == 0) {
        DEBUG("sched_run: Actual task: %d, SERVICE TIME ENDED! Exiting from current task! \n", (int)pid);
        sched_task_exit();
    }
}

void sched_set_status(thread_t *process, thread_status_t status)
{
    if (status >= STATUS_ON_RUNQUEUE) {
        if (!(process->status >= STATUS_ON_RUNQUEUE)) {
            DEBUG(
                "sched_set_status: adding thread %" PRIkernel_pid " to runqueue %" PRIu8 ".\n",
                process->pid, process->priority);
            clist_rpush(&sched_runqueues[process->priority],
                        &(process->rq_entry));
            _set_runqueue_bit(process);

                      for (int i = 0; i < SCHED_PRIO_LEVELS; i++) {
                          if ((int)clist_count(&sched_runqueues[i]) != 0)
        DEBUG(" -- SCHED_SET_STATUS_a -- CODA %d -> %d \n", i, (int)clist_count(&sched_runqueues[i]));
    }
        }
    }
    else {
        if (process->status >= STATUS_ON_RUNQUEUE) {
            DEBUG(
                "sched_set_status: removing thread %" PRIkernel_pid " from runqueue %" PRIu8 ".\n",
                process->pid, process->priority);
            clist_lpop(&sched_runqueues[process->priority]);
               for (int i = 0; i < SCHED_PRIO_LEVELS; i++) {
                          if ((int)clist_count(&sched_runqueues[i]) != 0)
        DEBUG(" -- SCHED_SET_STATUS_r -- CODA %d -> %d \n", i, (int)clist_count(&sched_runqueues[i]));
    }

            if (!sched_runqueues[process->priority].next) {
                _clear_runqueue_bit(process);
            }
        }
    }

    process->status = status;
}

void sched_switch(uint16_t other_prio)
{
    thread_t *active_thread = thread_get_active();
    uint16_t current_prio = active_thread->priority;
    int on_runqueue = (active_thread->status >= STATUS_ON_RUNQUEUE);

    DEBUG("sched_switch: active pid=%" PRIkernel_pid " prio=%" PRIu16 " on_runqueue=%i "
          ", other_prio=%" PRIu16 "\n",
          active_thread->pid, current_prio, on_runqueue,
          other_prio);

    if (!on_runqueue || (current_prio > other_prio)) {
        if (irq_is_in()) {
            DEBUG("sched_switch: setting sched_context_switch_request.\n");
            sched_context_switch_request = 1;
        }
        else {
            DEBUG("sched_switch: yielding immediately.\n");
            thread_yield_higher();
        } 
    }
    else {
        DEBUG("sched_switch: continuing without yield.\n");
    }
}

NORETURN void sched_task_exit(void)
{
    DEBUG("sched_task_exit: ending thread %" PRIkernel_pid "...\n",
          thread_getpid());

    (void)irq_disable();
    sched_threads[thread_getpid()] = NULL;
    sched_num_threads--;

    sched_set_status(thread_get_active(), STATUS_STOPPED);

    sched_active_thread = NULL;
    cpu_switch_context_exit();
}

#ifdef MODULE_SCHED_CB
void sched_register_cb(void (*callback)(kernel_pid_t, kernel_pid_t))
{
    sched_cb = callback;
}
#endif
