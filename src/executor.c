#include "queue.h"
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>

void* exec_worker(void *arg) {
	executor *exec = arg;
	exec_work *work;

	while(1) {
		pthread_mutex_lock(&(exec->work_mutex));

		while(peek(exec->work_queue) == NULL && !exec->stop) {
			// add cond_wait and rest of logic 
		}

	}
	return NULL;
}

executor* exec_create(size_t num) {
	executor *exec = malloc(sizeof(executor));
	if (exec == NULL) return NULL;
	pthread_t thread;
	size_t i;

	if (num == 0) {
		num = 2;
	}

	exec = calloc(1, sizeof(*exec));
	if (exec == NULL) return NULL;

	pthread_mutex_init(&(exec->work_mutex), NULL);
	pthread_cond_init(&(exec->work_cond), NULL);
	pthread_cond_init(&(exec->working_cond), NULL);

	exec->work_queue = queue_create(); // initialize queue.

	for (size_t i = 0; i < num; i++) {
		pthread_create(&thread, PTHREAD_CREATE_DETACHED, exec_worker, exec); 
		// tpool_worker()
	}
	return exec;
}

int exec_destroy(executor *exec) {
	return 0;
}

exec_work* exec_work_create(exec_func func, void *arg) {
	if (func == NULL) return NULL; // todo: err log
	
	exec_work *work = malloc(sizeof(exec_work));
	work->func = func;
	work->arg = arg;

	return work;
}

int exec_work_destroy(exec_work *work) {
	if (work == NULL) {
		return 1;
	}
	free(work);
	return 0;
}





bool exec_add_work(executor *exec, exec_func func, void *arg) {
	return true;
}

int exec_wait(executor *exec) {
	return 0;
}


