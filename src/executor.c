#include "queue.h"
#include "executor.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

executor* exec_create(size_t num) {
	if (num < 4) num = 4; // atleast 4 threads

	executor* exec = malloc(sizeof(executor));
	if (exec == NULL) return NULL;

	exec->threads = malloc(sizeof(pthread_t) * num);
	if (exec->threads == NULL) return NULL;

	pthread_mutex_init(&(exec->mutex), NULL);
	pthread_cond_init(&(exec->empty), NULL);
	pthread_cond_init(&(exec->non_empty), NULL);

	exec->working_count = 0;
	exec->pending_count = 0;
	exec->thread_count = num;
	exec->stop = false;

	exec->queue = queue_create();

	for (size_t i = 0; i < num; i++) {
		pthread_t thread;
		if (pthread_create(&thread, NULL, exec_worker, exec) != 0) {
			pthread_mutex_lock(&(exec->mutex));

			exec->stop = true;
			pthread_cond_broadcast(&(exec->non_empty));

			pthread_mutex_unlock(&(exec->mutex));
			for (size_t j = 0; j < i; j++) {
				pthread_join(exec->threads[j], NULL);
			}

			pthread_mutex_destroy(&(exec->mutex));
			pthread_cond_destroy(&(exec->empty));
			pthread_cond_destroy(&(exec->non_empty));

			queue_destroy(exec->queue);
			free(exec->threads);
			free(exec);
			
			exit(2); // temp: remove it later 
		}
		exec->threads[i] = thread;
	}

	
	return exec;
}

int exec_destroy(executor *exec) {
	if (!exec) return 1;

	

	pthread_mutex_lock(&(exec->mutex));

	exec->stop = true;
	pthread_cond_broadcast(&(exec->non_empty));

	pthread_mutex_unlock(&(exec->mutex));

	for (size_t i = 0; i < exec->thread_count; i++) {
		pthread_join(exec->threads[i], NULL);
		
	}
	pthread_mutex_destroy(&(exec->mutex));
	pthread_cond_destroy(&(exec->empty));
	pthread_cond_destroy(&(exec->non_empty));

	queue_destroy(exec->queue);

	free(exec->threads);
	free(exec);

	
	return 0; // success yay
}

bool exec_add_work(executor *exec, exec_func func, void *arg) {
	if (!exec || !func) return false;
	pthread_mutex_lock(&(exec->mutex));

	if (exec->stop) {
		pthread_mutex_unlock(&(exec->mutex));
		return false;
	}
	exec_work* work = exec_work_create(func, arg);
	if (work == NULL) {
		pthread_mutex_unlock(&(exec->mutex));
		return false;
	}
	enqueue(exec->queue, &work, sizeof(exec_work));
	free(work);
	exec->pending_count++;
	pthread_cond_signal(&(exec->non_empty));

	pthread_mutex_unlock(&(exec->mutex));
	return true;
}



int exec_wait(executor *exec) {
	if (!exec) return 1;

	pthread_mutex_lock(&(exec->mutex));
	while (exec->pending_count > 0 || exec->working_count > 0) { // while work exists
		
		pthread_cond_wait(&(exec->empty), &(exec->mutex));
	}
	pthread_mutex_unlock(&(exec->mutex));

	return 0;
}

void* exec_worker(void *arg) {
	if (!arg) return NULL;
	executor* exec = arg;
	exec_work* work;
	while (1) {
		pthread_mutex_lock(&(exec->mutex));

		while(is_empty(exec->queue) && !exec->stop) {
			pthread_cond_wait(&(exec->non_empty), &(exec->mutex));
		}

		if (exec->stop && is_empty(exec->queue)) {
			pthread_mutex_unlock(&(exec->mutex));
			break; // terminate loop
		}

		
		work = (exec_work *)dequeue(exec->queue);

		exec->pending_count--;
		exec->working_count++;

		pthread_mutex_unlock(&(exec->mutex));

		if (work) {
			
			work->func(work->arg);
			exec_work_destroy(work);
		} else {
			
		}
		pthread_mutex_lock(&(exec->mutex));
		exec->working_count--;
		if (exec->working_count == 0 && exec->pending_count == 0) {
			pthread_cond_signal(&(exec->empty));
		}
		pthread_mutex_unlock(&(exec->mutex));
	}
	return NULL;
}

exec_work* exec_work_create(exec_func func, void *arg) {
	if (!func) return NULL;

	exec_work* work = malloc(sizeof(exec_work));
	if (!work) return NULL;

	work->func = func;
	work->arg = arg;

	return work;
}

int exec_work_destroy(exec_work *work) {
	if (work == NULL) return 1;
	free(work);
	return 0;
}

