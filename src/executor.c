#include "queue.h"
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>

executor* exec_create(size_t num) {

	executor *exec = malloc(sizeof(executor));

	if (exec == NULL) return NULL;

	pthread_t thread;
	pthread_attr_t attr;
	size_t i;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (num  <= 3) {
		num = 4; // atmost 4 threads
	}

	exec->thread_count = num;

	if (exec == NULL) return NULL;

	pthread_mutex_init(&(exec->work_mutex), NULL);
	pthread_cond_init(&(exec->work_cond), NULL);
	pthread_cond_init(&(exec->working_cond), NULL);

	exec->work_queue = queue_create(); // initialize queue.

	for (size_t i = 0; i < num; i++) {
		pthread_create(&thread, &attr, exec_worker, exec); 
	}

	fprintf(stdout, "[exec_create()]: executor initialized\n");
	return exec;
}

int exec_destroy(executor *exec) {
	if (exec == NULL) return 1;

	pthread_mutex_lock(&(exec->work_mutex));
	exec->stop = true;

	pthread_cond_broadcast(&(exec->work_cond));
	pthread_mutex_unlock(&(exec->work_mutex));

	if ((exec_wait(exec)) != 0) {
		return 1;
	}

	pthread_mutex_destroy(&(exec->work_mutex));
	pthread_cond_destroy(&(exec->working_cond));
	pthread_cond_destroy(&(exec->work_cond));

	queue_destroy(exec->work_queue);
	free(exec);

	fprintf(stdout, "[exec_destroy()]: executor destroyed\n");

	return 0;
}

exec_work* exec_work_create(exec_func func, void *arg) {
	if (func == NULL) return NULL; // todo: err log

	exec_work *work = malloc(sizeof(exec_work));
	work->func = func;
	work->arg = arg;

	fprintf(stdout, "[exec_work_create()]: work object created\n");
	return work;
}

int exec_work_destroy(exec_work *work) {
	if (work == NULL) {
		return 1;
	}
	free(work);
	fprintf(stdout, "[exec_work_destroy()]: work object destroyed\n");
	return 0;
}

int exec_wait(executor *exec) {
	if (exec == NULL) return 0; // bug: might cause segfault later

	pthread_mutex_lock(&(exec->work_mutex));

	while (((exec->working_count > 0) || !is_empty(exec->work_queue))) {
		if (exec->stop) {
			break;
		}
		fprintf(stdout, "[exec_wait::DEBUG]: waitiing..\n");
		pthread_cond_wait(&(exec->working_cond),&(exec->work_mutex));
	}

	pthread_mutex_unlock(&(exec->work_mutex));
	return 0;
}

bool exec_add_work(executor *exec, exec_func func, void *arg) {
	if (exec == NULL) return false;
	pthread_mutex_lock(&(exec->work_mutex));

	exec_work *work = exec_work_create(func, arg);

	if (work == NULL) { 
		pthread_mutex_unlock(&(exec->work_mutex));
		return false; // todo: err handler  
	}

	enqueue(exec->work_queue, work, sizeof(exec_work));

	pthread_cond_broadcast(&(exec->working_cond));
	pthread_mutex_unlock(&(exec->work_mutex));

	return true;
}

void* exec_worker(void *arg) {
	executor *exec = arg;
	exec_work *work;

	while(1) {
		pthread_mutex_lock(&(exec->work_mutex));

		while(!exec->stop && is_empty(exec->work_queue)) {
			fprintf(stdout, "[exec_worker::DEBUG]: waiting..\n");
			pthread_cond_wait(&(exec->work_cond), &(exec->work_mutex));
		}
		if (exec->stop) break;

		work = (exec_work *)peek(exec->work_queue);
		exec->working_count++;

		pthread_mutex_unlock(&(exec->work_mutex));

		if (!work) {
			fprintf(stderr, "[err]: failed to fetch work object\n");
		} else {
			dequeue(exec->work_queue);
			fprintf(stdout, "[executor]: work executing\n");
			work->func(work->arg); // execute the func with args
			fprintf(stdout, "[executor]: work executed\n");
			exec_work_destroy(work);
		}

		pthread_mutex_lock(&(exec->work_mutex));
		exec->working_count--;
		if (!exec->stop && exec->working_count == 0 && !is_empty(exec->work_queue)) {
			pthread_cond_signal(&(exec->working_cond)); // sleeps when working counts is 0 and queue is empty
			fprintf(stdout, "[executor]: queue is empty and working objects are none\ngoing to sleep...\n");
		}
		pthread_mutex_unlock(&(exec->work_mutex));
	}

	exec->thread_count--;
	pthread_cond_signal(&(exec->working_cond));
	pthread_mutex_unlock(&(exec->work_mutex));

	return NULL;
}
