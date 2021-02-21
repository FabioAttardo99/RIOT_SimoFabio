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
#include "timex.h"
#include "mutex.h"

#define THREAD_NUMOF (7U)
#define THREAD_FIRSTGROUP_NUMOF (3U)
//static char stacks[THREAD_NUMOF][THREAD_STACKSIZE_MAIN];
//static const char prios[THREAD_NUMOF] = {THREAD_PRIORITY_MAIN - 1, 4, 3, 2, 1};


char t1_stack[THREAD_STACKSIZE_MAIN];
char t2_stack[THREAD_STACKSIZE_MAIN];
char t3_stack[THREAD_STACKSIZE_MAIN];
kernel_pid_t p_main, p1, p2, p3;

// static mutex_t lock;

void *thread1(void *arg)
{
//	printf("TIME: %d", xtimer_now_usec());
//	mutex_lock(&lock);
	(void) arg;
	thread_t *t = thread_get_active();
   	puts("++ Prima stringa ++");
   	printf("PID: %d, priority: %d time: %d \n", (int)t->pid, (int)t->priority, (int)t->s_time);
//	mutex_unlock(&lock);
   	return NULL;
}

void *thread2(void *arg)
{
//	printf("TIME: %d", xtimer_now_usec());
//	mutex_lock(&lock);
	(void) arg;
  	thread_t *t1 = thread_get_active();
	puts("++ seconda stringa ++");
   	printf("PID: %d, priority: %d time: %d \n", (int)t1->pid, (int)t1->priority, (int)t1->s_time);
//	mutex_unlock(&lock);
    return NULL;
}

void *thread3(void *arg)
{
//	printf("TIME: %d", xtimer_now_usec());
//	mutex_lock(&lock);
	(void) arg;
  	thread_t *t2 = thread_get_active();
	puts("++ terza stringa ++");
   	printf("PID: %d, priority: %d time: %d \n", (int)t2->pid, (int)t2->priority, (int)t2->s_time);
//	mutex_unlock(&lock);
    	return NULL;
}

void Saluto(int var)
{
	printf("Sono bello %d" ,var);
	
}

int main(void) {
  	
  	/*
  	xtimer_ticks32_t ret;
  	ret.ticks32 = _xtimer_now();
  	printf("ret: %d", (int)ret.ticks32);
  	*/
  	uint32_t Time = 500;
	xtimer_t Run;
	Run.next = NULL;         /**< reference to next timer in timer lists */
    Run.offset = 0;             /**< lower 32bit offset time */
    Run.long_offset = 0;        /**< upper 32bit offset time */
    Run.start_time = 0;         /**< lower 32bit absolute start time */
    Run.long_start_time = 0;    /**< upper 32bit absolute start time */
    Run.callback = Saluto;
	Run.arg = (int*); 
	
	
	// printf("Time: %d  \n" , xtimer_usec_from_ticks(xtimer_ticks(Time)));
	xtimer_set(&Run,Time);

	p1 = thread_create(t1_stack, sizeof(t1_stack), 8, 0, thread1, NULL, "nr1", 3000);
	p2 = thread_create(t2_stack, sizeof(t2_stack), 8, 0, thread2, NULL, "nr2", 1000);
	p3 = thread_create(t3_stack, sizeof(t3_stack), 8, 0, thread3, NULL, "nr3", 500);

	
                       
//	mutex_unlock(&lock); 

//	mutex_lock(&lock);
                       /*
	for (unsigned i = 0; i < THREAD_NUMOF; i++) {
	printf("Sto creando il processo!");
        thread_create(stacks[i], sizeof(stacks[i]), prios[i], 0, prova, NULL, "t");
        printf("Ho creato il processo!");*/
    
}
