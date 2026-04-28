#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "executor.h"


static const size_t threads = 4;
static const size_t items = 10;

void worker (void *arg) {
	int *val = arg;
	int old = *val;
	*val += 1;
	printf("tid=%p, old=%d, val=%d\n", (void*)pthread_self(), old, *val);

	if (*val % 2) {
		usleep(1000);
	}
	return;
}

int main(int argc, char **argv) {
	executor *exec;
	int *vals;
	size_t i;
	printf("[main]: main func\n");
	exec = exec_create(threads);

	vals = calloc(items, sizeof(*vals));
	for (i = 0; i < items; i++) {
		vals[i] = i;
		exec_add_work(exec, worker, vals + i);
	}

	exec_wait(exec);

	for (i = 0; i < items; i++) {
		printf("%d\n", vals[i]);
	}
	free(vals);
	printf("[end]: yay\n");
	exec_destroy(exec);
	printf("[end]: yay\n");
	return 0;

}
