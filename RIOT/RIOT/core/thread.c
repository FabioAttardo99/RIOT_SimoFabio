/*
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     core_thread
 * @{
 *
 * @file
 * @brief       Threading implementation
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <errno.h>
#include <stdio.h>
#ifdef PICOLIBC_TLS
#include <picotls.h>
#endif

#include "assert.h"
#include "thread.h"
#include "irq.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"
#include "bitarithm.h"
#include "sched.h"

thread_status_t thread_getstatus(kernel_pid_t pid)
{
    thread_t *thread = thread_get(pid);

    return thread ? thread->status : STATUS_NOT_FOUND;
}

const char *thread_getname(kernel_pid_t pid)
{
#ifdef CONFIG_THREAD_NAMES
    thread_t *thread = thread_get(pid);
    return thread ? thread->name : NULL;
#else
    (void)pid;
    return NULL;
#endif
}

void thread_zombify(void)
{
    if (irq_is_in()) {
        return;
    }

    irq_disable();
    sched_set_status(thread_get_active(), STATUS_ZOMBIE);
    irq_enable();
    thread_yield_higher();

    /* this line should never be reached */
    UNREACHABLE();
}

int thread_kill_zombie(kernel_pid_t pid)
{
    DEBUG("thread_kill: Trying to kill PID %" PRIkernel_pid "...\n", pid);
    unsigned state = irq_disable();

    int result = (int)STATUS_NOT_FOUND;

    thread_t *thread = thread_get(pid);

    if (!thread) {
        DEBUG("thread_kill: Thread does not exist!\n");
    }
    else if (thread->status == STATUS_ZOMBIE) {
        DEBUG("thread_kill: Thread is a zombie.\n");

        sched_threads[pid] = NULL;
        sched_num_threads--;
        sched_set_status(thread, STATUS_STOPPED);

        result =  1;
    }
    else {
        DEBUG("thread_kill: Thread is not a zombie!\n");
    }
    irq_restore(state);
    return result;
}

void thread_sleep(void)
{
    if (irq_is_in()) {
        return;
    }

    unsigned state = irq_disable();
    sched_set_status(thread_get_active(), STATUS_SLEEPING);
    irq_restore(state);
    thread_yield_higher();
}

int thread_wakeup(kernel_pid_t pid)
{
    DEBUG("thread_wakeup: Trying to wakeup PID %" PRIkernel_pid "...\n", pid);

    unsigned old_state = irq_disable();

    thread_t *thread = thread_get(pid);

    if (!thread) {
        DEBUG("thread_wakeup: Thread does not exist!\n");
    }
    else if (thread->status == STATUS_SLEEPING) {
        DEBUG("thread_wakeup: Thread is sleeping.\n");

        sched_set_status(thread, STATUS_RUNNING);

        irq_restore(old_state);
        sched_switch(thread->priority);

        return 1;
    }
    else {
        DEBUG("thread_wakeup: Thread is not sleeping!\n");
    }

    irq_restore(old_state);
    return (int)STATUS_NOT_FOUND;
}


void thread_yield(void)
{
    unsigned old_state = irq_disable();
    thread_t *me = thread_get_active();

    if (me->status >= STATUS_ON_RUNQUEUE) {
        clist_lpoprpush(&sched_runqueues[me->priority]);
    }
    irq_restore(old_state);

    thread_yield_higher();
}

void thread_add_to_list(list_node_t  *list, thread_t *thread)
{
    assert(thread->status < STATUS_ON_RUNQUEUE);

    uint16_t my_prio = thread->priority;
    list_node_t *new_node = (list_node_t *)&thread->rq_entry;

    while (list->next) {
        thread_t *list_entry = container_of((clist_node_t *)list->next,
                                            thread_t, rq_entry);
        if (list_entry->priority > my_prio) {
            break;
        }
        list = list->next;
    }

    new_node->next = list->next;
    list->next = new_node;
}

#ifdef DEVELHELP
uintptr_t thread_measure_stack_free(const char *stack)
{
    /* Alignment of stack has been fixed (if needed) by thread_create(), so
     * we can silence -Wcast-align here */
    uintptr_t *stackp = (uintptr_t *)(uintptr_t)stack;

    /* assume that the comparison fails before or after end of stack */
    /* assume that the stack grows "downwards" */
    while (*stackp == (uintptr_t)stackp) {
        stackp++;
    }

    uintptr_t space_free = (uintptr_t)stackp - (uintptr_t)stack;
    return space_free;
}
#endif

kernel_pid_t thread_create(char *stack, int stacksize, uint8_t priority,
                           int flags, thread_task_func_t function, void *arg,
                           const char *name, int s_time)
{
    if (priority >= SCHED_PRIO_LEVELS) {
        return -EINVAL;
    }

#ifdef DEVELHELP
    int total_stacksize = stacksize;
#endif
#ifndef CONFIG_THREAD_NAMES
    (void) name;
#endif

    /* align the stack on a 16/32bit boundary */
    uintptr_t misalignment = (uintptr_t)stack % ALIGN_OF(void *);
    if (misalignment) {
        misalignment = ALIGN_OF(void *) - misalignment;
        stack += misalignment;
        stacksize -= misalignment;
    }

    /* make room for the thread control block */
    stacksize -= sizeof(thread_t);

    /* round down the stacksize to a multiple of thread_t alignments (usually 16/32bit) */
    stacksize -= stacksize % ALIGN_OF(thread_t);

    if (stacksize < 0) {
        DEBUG("thread_create: stacksize is too small!\n");
    }
    /* allocate our thread control block at the top of our stackspace. Cast to
     * (uintptr_t) intermediately to silence -Wcast-align. (We manually made
     * sure alignment is correct above.) */
    thread_t *thread = (thread_t *)(uintptr_t)(stack + stacksize);

#ifdef PICOLIBC_TLS
    stacksize -= _tls_size();

    thread->tls = stack + stacksize;
    _init_tls(thread->tls);
#endif

#if defined(DEVELHELP) || defined(SCHED_TEST_STACK)
    if (flags & THREAD_CREATE_STACKTEST) {
        /* assign each int of the stack the value of it's address. Alignment
         * has been handled above, so silence -Wcast-align */
        uintptr_t *stackmax = (uintptr_t *)(uintptr_t)(stack + stacksize);
        uintptr_t *stackp = (uintptr_t *)(uintptr_t)stack;

        while (stackp < stackmax) {
            *stackp = (uintptr_t)stackp;
            stackp++;
        }
    }
    else {
        /* create stack guard. Alignment has been handled above, so silence
         * -Wcast-align */
        *(uintptr_t *)(uintptr_t)stack = (uintptr_t)stack;
    }
#endif

    unsigned state = irq_disable();

    kernel_pid_t pid = KERNEL_PID_UNDEF;
    for (kernel_pid_t i = KERNEL_PID_FIRST; i <= KERNEL_PID_LAST; ++i) {
        if (sched_threads[i] == NULL) {
            pid = i;
            break;
        }
    }
    if (pid == KERNEL_PID_UNDEF) {
        DEBUG("thread_create(): too many threads!\n");

        irq_restore(state);

        return -EOVERFLOW;
    }

    sched_threads[pid] = thread;
    // Imposto la service time del thread
    thread->s_time = s_time;
    thread->pid = pid;
    thread->sp = thread_stack_init(function, arg, stack, stacksize);

#if defined(DEVELHELP) || defined(SCHED_TEST_STACK) || \
    defined(MODULE_MPU_STACK_GUARD)
    thread->stack_start = stack;
#endif

#ifdef DEVELHELP
    thread->stack_size = total_stacksize;
#endif
#ifdef CONFIG_THREAD_NAMES
    thread->name = name;
#endif

    if ((int)pid == 1) thread->priority = 15;// versione originale     thread->priority = priority;
    else thread->priority = 1;
    thread->status = STATUS_STOPPED;

    thread->rq_entry.next = NULL;

#ifdef MODULE_CORE_MSG
    thread->wait_data = NULL;
    thread->msg_waiters.next = NULL;
    cib_init(&(thread->msg_queue), 0);
    thread->msg_array = NULL;
#endif

    sched_num_threads++;

    DEBUG("Created thread %s. PID: %" PRIkernel_pid ". Priority: %u.\n", name,
          thread->pid, priority);



    if (flags & THREAD_CREATE_SLEEPING) {
        sched_set_status(thread, STATUS_SLEEPING);
    }
    else {
        sched_set_status(thread, STATUS_PENDING);

        if (!(flags & THREAD_CREATE_WOUT_YIELD)) {
            irq_restore(state);
            sched_switch(priority);
            return pid;
        }
    }

    irq_restore(state);

    return pid;
}
