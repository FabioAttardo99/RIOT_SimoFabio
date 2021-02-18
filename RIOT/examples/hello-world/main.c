/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 *

#include <stdio.h>

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    return 0;
}
*/
#include <stdio.h>
#include <inttypes.h>



#include "thread.h"
#include "msg.h"
#include "xtimer.h"

#define THREAD_NUMOF (5U)
#define THREAD_FIRSTGROUP_NUMOF (3U)
//static char stacks[THREAD_NUMOF][THREAD_STACKSIZE_MAIN];
//static const char prios[THREAD_NUMOF] = {THREAD_PRIORITY_MAIN - 1, 4, 3, 2, 1};


char t1_stack[THREAD_STACKSIZE_MAIN];
char t2_stack[THREAD_STACKSIZE_MAIN];
kernel_pid_t p_main, p1, p2, p3;
/*
static void *prova(void *arg)
{

	(void)arg;
   	thread_t *t = thread_get_active();
	printf("T%i (prio %i): sono proprio io!\n",
           (int)t->pid, (int)t->priority);
           
        thread_wakeup(t->pid);
           
        thread_yield();
           
           
           if(xtimer_now_usec(void) < 3000)
           {
	          return NULL;           
           }
           
        	

}
*/


void *thread1(void *arg)
{
	(void) arg;
    puts("Prima stringa\n");
    return NULL;
}

void *thread2(void *arg)
{
	(void) arg;
   puts("seconda stringa\n");
   printf("%d", (int)xtimer_now());

    return NULL;
}


int main(void) {

  	//xtimer_init();
	
	p1 = thread_create(t1_stack, sizeof(t1_stack), THREAD_PRIORITY_MAIN - 1,
                       THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                       thread1, NULL, "nr1");
	p2 = thread_create(t2_stack, sizeof(t2_stack), THREAD_PRIORITY_MAIN - 1,
                       THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                       thread2, NULL, "nr2");
                       
                       /*
	for (unsigned i = 0; i < THREAD_NUMOF; i++) {
	printf("Sto creando il processo!");
        thread_create(stacks[i], sizeof(stacks[i]), prios[i], 0, prova, NULL, "t");
        printf("Ho creato il processo!");*/
    
}
