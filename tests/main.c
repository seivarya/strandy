#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "executor.h"


static const size_t threads = 4;
static const size_t items = 40;

void worker (void *arg) {
	printf("[call]: worker method called\n");
	int *val = arg;
	int old = *val;
	*val += 10000;
	printf("tid=%p, old=%d, val=%d\n", (void*)pthread_self(), old, *val);

	if (*val % 2) {
		usleep(100000);
	printf("[call]: worker on sleep\n");
	}
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
		fprintf(stdout, "work inserted [%zu]\n", i);
	}

	exec_wait(exec);

	for (i = 0; i < items; i++) {
		printf("%d\n", vals[i]);
	}

	free(vals);
	exec_destroy(exec);
	return 0;

}
