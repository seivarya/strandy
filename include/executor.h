#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "queue.h"
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>

typedef void (*exec_func)(void *arg);

typedef struct exec_work {

	exec_func func;
	void *arg;

} exec_work;


typedef struct executor {

	queue *work_queue;
	pthread_mutex_t work_mutex;
	pthread_cond_t work_cond; // there are threads in queue waiting to be processed
	pthread_cond_t working_cond; // there are no threads to be processed!
	size_t working_count; // number of threads actively being processed.
	size_t thread_count; // number of alive threads
	bool stop;

} executor;

executor* exec_create(size_t num);
int exec_destroy(executor *exec);

exec_work* exec_work_create(exec_func func, void *arg);
int exec_work_destroy(exec_work *work);

bool exec_add_work(executor *exec, exec_func func, void *arg);
int exec_wait(executor *exec);

void* exec_worker(void *arg);


#endif
