/* L'applicazione in questione si occupa di realizzare 5 thread, ciascuno avente uno specifico
 * service time. Lo scheduler si occuperà, quindi, di variare l'esecuzione di tutti i thread fino
 * a quando la service time non sarà conclusa per ognuno di essi. 
 */


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

/* Queste variabili servono per permettere al thread di stampare a video solo un messaggio ad
 * esecuzione. Se la variabile associata ad un thread ha valore pari ad 1 allora quel thread
 * può stampare un messaggio. Se, invece, la variabile ha un valore impostato a 0, quel
 * thread non può stampare alcun messaggio.
 */ 

static int to_write1 = 1;
static int to_write2 = 1;
static int to_write3 = 1;
static int to_write4 = 1;
static int to_write5 = 1;

/*** CODICE DESCRITTIVO DEI THREAD ***/

void *thread1(void *arg)
{
	(void) arg;
	thread_t *t = thread_get_active();
   	
	/* Il ciclo while in questione si occupa di ciclare un singolo thread e di stampare, una
	 * volta soltanto, un messaggio che indica quale sia il thread in esecuzione. Lo stesso
	 * messaggio include anche un'informazione sulla service time rimanente, espressa in 
	 * secondi. Le variabili poste al di sotto sono appositamente impostate in modo da consentire
	 * una sola 'printf'.
	 */   
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

	/* Il ciclo while in questione si occupa di ciclare un singolo thread e di stampare, una
	 * volta soltanto, un messaggio che indica quale sia il thread in esecuzione. Lo stesso
	 * messaggio include anche un'informazione sulla service time rimanente, espressa in 
	 * secondi. Le variabili poste al di sotto sono appositamente impostate in modo da consentire
	 * una sola 'printf'.
	 */     
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
	
	/* Il ciclo while in questione si occupa di ciclare un singolo thread e di stampare, una
	 * volta soltanto, un messaggio che indica quale sia il thread in esecuzione. Lo stesso
	 * messaggio include anche un'informazione sulla service time rimanente, espressa in 
	 * secondi. Le variabili poste al di sotto sono appositamente impostate in modo da consentire
	 * una sola 'printf'.
	 */   
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

	/* Il ciclo while in questione si occupa di ciclare un singolo thread e di stampare, una
	 * volta soltanto, un messaggio che indica quale sia il thread in esecuzione. Lo stesso
	 * messaggio include anche un'informazione sulla service time rimanente, espressa in 
	 * secondi. Le variabili poste al di sotto sono appositamente impostate in modo da consentire
	 * una sola 'printf'.
	 */   
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

	/* Il ciclo while in questione si occupa di ciclare un singolo thread e di stampare, una
	 * volta soltanto, un messaggio che indica quale sia il thread in esecuzione. Lo stesso
	 * messaggio include anche un'informazione sulla service time rimanente, espressa in 
	 * secondi. Le variabili poste al di sotto sono appositamente impostate in modo da consentire
	 * una sola 'printf'.
	 */     
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

/*** FINE CODICE DESCRITTIVO DEI THREAD ***/

/*** CREAZIONE DEI THREAD - l'ultimo campo indica la service time impostata ***/
int main(void) {
	p1 = thread_create(t1_stack, sizeof(t1_stack), 8, 0, thread1, NULL, "nr1", 3000);
	p2 = thread_create(t2_stack, sizeof(t2_stack), 8, 0, thread2, NULL, "nr2", 6000);
	p3 = thread_create(t3_stack, sizeof(t3_stack), 8, 0, thread3, NULL, "nr3", 4000);
	p4 = thread_create(t4_stack, sizeof(t4_stack), 8, 0, thread4, NULL, "nr4", 5000);
	p5 = thread_create(t5_stack, sizeof(t5_stack), 8, 0, thread5, NULL, "nr5", 2000);
}
/*** FINE CREAZIONE DEI THREAD ***/