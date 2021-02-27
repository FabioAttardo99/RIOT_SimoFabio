#include <stdio.h>
#include <inttypes.h>
#include "thread.h"
#include "msg.h"
#include "xtimer.h"
#include "timex.h"
#include "mutex.h"
#define THREAD_NUMOF (7U)
#define THREAD_FIRSTGROUP_NUMOF (5U)

char t1_stack[THREAD_STACKSIZE_MAIN];
char t2_stack[THREAD_STACKSIZE_MAIN];
char t3_stack[THREAD_STACKSIZE_MAIN];
char t4_stack[THREAD_STACKSIZE_MAIN];
char t5_stack[THREAD_STACKSIZE_MAIN];
kernel_pid_t p_main, p1, p2, p3, p4, p5;

static int to_write1 = 1;
static int to_write2 = 1;
static int to_write3 = 1;
static int to_write4 = 1;
static int to_write5 = 1;

void *thread1(void *arg)
{
	(void) arg;
	thread_t *t = thread_get_active();
   	
	while (1) 
	{
		if (to_write1 == 1) {
			printf("THREAD NAME: %s, REMAINING SERVICE TIME: %f seconds\n", t->name, (double)(t->s_time)/1000);
			to_write1 = 0;
			to_write2 = 1;
			to_write3 = 1;
			to_write4 = 1;
			to_write5 = 1;
		}
	}
   	return NULL;
}

void *thread2(void *arg)
{
	(void) arg;
  	thread_t *t1 = thread_get_active();
   	while (1) 
	{
		if (to_write2 == 1) {
			printf("THREAD NAME: %s, REMAINING SERVICE TIME: %f seconds\n", t1->name, (double)(t1->s_time)/1000);
			to_write2 = 0;
			to_write1 = 1;
			to_write3 = 1;
			to_write4 = 1;
			to_write5 = 1;
		}
	}
    return NULL;
}

void *thread3(void *arg)
{
	(void) arg;
  	thread_t *t2 = thread_get_active();
   	while (1) 
	{
		if (to_write3 == 1) {
			printf("THREAD NAME: %s, REMAINING SERVICE TIME: %f seconds\n", t2->name, (double)(t2->s_time)/1000);
			to_write3 = 0;
			to_write1 = 1;
			to_write2 = 1;
			to_write4 = 1;
			to_write5 = 1;
		}
	}
    return NULL;
}

void *thread4(void *arg)
{
	(void) arg;
  	thread_t *t3 = thread_get_active();
    while (1) 
	{
		if (to_write4 == 1) {
			printf("THREAD NAME: %s, REMAINING SERVICE TIME: %f seconds\n", t3->name, (double)(t3->s_time)/1000);
			to_write4 = 0;
			to_write2 = 1;
			to_write3 = 1;
			to_write1 = 1;
			to_write5 = 1;
		}
	}
    return NULL;
}

void *thread5(void *arg)
{
	(void) arg;
  	thread_t *t4 = thread_get_active();
   	while (1) 
	{
		if (to_write5 == 1) {
			printf("THREAD NAME: %s, REMAINING SERVICE TIME: %f seconds\n", t4->name, (double)(t4->s_time)/1000);
			to_write5 = 0;
			to_write2 = 1;
			to_write3 = 1;
			to_write4 = 1;
			to_write1 = 1;
		}
	}
    return NULL;
}

int main(void) {
	p1 = thread_create(t1_stack, sizeof(t1_stack), 8, 0, thread1, NULL, "nr1", 3000);
	p2 = thread_create(t2_stack, sizeof(t2_stack), 8, 0, thread2, NULL, "nr2", 6000);
	p3 = thread_create(t3_stack, sizeof(t3_stack), 8, 0, thread3, NULL, "nr3", 4000);
	p4 = thread_create(t4_stack, sizeof(t4_stack), 8, 0, thread4, NULL, "nr4", 5000);
	p5 = thread_create(t5_stack, sizeof(t5_stack), 8, 0, thread5, NULL, "nr5", 2000);
}