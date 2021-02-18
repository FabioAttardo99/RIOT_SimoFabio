#include <stdio.h>
#include "thread.h"

#define THREAD_NUMOF (5U)
static char stacks[THREAD_NUMOF][THREAD_STACKSIZE_MAIN];
static const char prios[THREAD_NUMOF] = {5, 5, 5, 5, 5};


static void *prova(void *arg)
{
	printf("T%i (prio %i): sono proprio io!\n",
           (int)t->pid, (int)t->priority);
}

int main() {
	for (int i = 0; i < 5; i++) {
        thread_create(stacks[i], sizeof(stacks[i]), prios[i], 0, prova, NULL, "t");
    }
}
