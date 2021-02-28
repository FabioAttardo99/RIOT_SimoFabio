/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *               2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     core_internal
 * @{
 *
 * @file
 * @brief       Platform-independent kernel initialization
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "kernel_init.h"
#include "thread.h"
#include "irq.h"
#include "log.h"

#include "periph/pm.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#ifdef MODULE_AUTO_INIT
#include <auto_init.h>
#endif

#ifndef CONFIG_BOOT_MSG_STRING
#define CONFIG_BOOT_MSG_STRING "main(): This is RIOT! (Version: " RIOT_VERSION ")"
#endif

extern int main(void);

static void *main_trampoline(void *arg)
{
    (void)arg;

#ifdef MODULE_AUTO_INIT
    auto_init();
#endif

    if (!IS_ACTIVE(CONFIG_SKIP_BOOT_MSG)) {
        LOG_INFO(CONFIG_BOOT_MSG_STRING "\n");
    }

    main();

    return NULL;
}

static char main_stack[THREAD_STACKSIZE_MAIN];
static char idle_stack[THREAD_STACKSIZE_IDLE];

/* Questa variabile serve per permettere al thread di stampare a video solo un messaggio ad
 * esecuzione. Se la variabile associata al thread idle ha valore pari ad 1 allora quel thread
 * può stampare un messaggio. Se, invece, la variabile ha un valore impostato a 0, quel
 * thread non può stampare alcun messaggio.
 */ 
static int idle_print = 1;

static void *idle_thread(void *arg)
{
    (void)arg;
    thread_t *idle = thread_get_active();

    /* Il ciclo while in questione si occupa di ciclare un singolo thread e di stampare, una
	 * volta soltanto, un messaggio che indica quale sia il thread in esecuzione. Lo stesso
	 * messaggio include anche un'informazione sulla service time rimanente, espressa in 
	 * secondi.
	 */     

    while (1) {
        if (idle_print == 1) {
            printf("THREAD NAME: %s, REMAINING SERVICE TIME: %d \n", idle->name, idle->s_time);
            idle_print = 0;
        }
        pm_set_lowest();
    }

    return NULL;
}



void kernel_init(void)
{
    irq_disable();

    if (IS_USED(MODULE_CORE_IDLE_THREAD)) {
        thread_create(idle_stack, sizeof(idle_stack),
                      THREAD_PRIORITY_IDLE,
                      THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                      idle_thread, NULL, "idle", 0);
    }

    thread_create(main_stack, sizeof(main_stack),
                  THREAD_PRIORITY_MAIN,
                  THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                  main_trampoline, NULL, "main", 0);

    cpu_switch_context_exit();
}
