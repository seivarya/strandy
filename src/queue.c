#include "queue.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

queue* queue_create() {
	queue *q = malloc(sizeof(queue));
	if (!q) {
		return NULL;
	}

	q->head = NULL;
	q->tail = NULL;
	q->length = 0;

	pthread_mutex_init(q->lock, NULL);
	pthread_cond_init(q->cond, NULL);

	return q;
}

int queue_destroy(queue *q) {
	pthread_mutex_lock(q->lock);
	if (!q) {
		return 1;
	}

	node *cur = q->head;
	while (cur) {
		node *next = cur->next;
		node_destroy(cur);
		cur = next;
	} 

	// todo: destroy the mutex & cond & queue itself
	
	pthread_mutex_unlock(q->lock);
	free(q);
	return 0;
}

int enqueue(queue *q, void *data, size_t size) {
	pthread_mutex_lock(q->lock);
	if (!q || !data || size == 0) {
		return 1;
	}

	node *n = node_create(data, size);
	if (!n) {
		return 1;
	}

	if (!q->head) {
		q->head = n;
		q->tail = n;
	} else {
		q->tail->next = n;
		q->tail = n;
	}

	q->length++;

	pthread_cond_signal(q->cond);
	pthread_mutex_unlock(q->lock);

	return 0;
}

node* dequeue(queue *q) { //  BUG:  dequeue removes the node and *peek returns it 2 opeartions might cause race conditions
	
	pthread_mutex_lock(q->lock);

	if (q->length == 0) {
		pthread_cond_wait(q->cond, q->lock);
	}

	if (!q || !q->head) {
		pthread_mutex_unlock(q->lock);
		return NULL;
	}

	node *n = q->head;
	q->head = q->head->next;

	if (!q->head) {
		q->tail = NULL;
	}

	q->length--;

	pthread_mutex_unlock(q->lock);
	return n;
}

int is_empty(queue *q) {
	return q && q->length == 0; // mutex lock unlock not needed here since it's atomic op
}

void* peek(queue *q) {
	pthread_mutex_lock(q->lock);
	if (!q || !q->head) {
		pthread_mutex_unlock(q->lock);
		return NULL;
	}
	pthread_mutex_unlock(q->lock);
	return q->head->data; // perhaps i should store data in a variable first then unlock mutex & return?
}

node* node_create(void *data, size_t size) {
	if (!data || size == 0) {
		return NULL;
	}

	node *n = malloc(sizeof(node));
	if (!n) {
		return NULL;
	}

	n->data = malloc(size);
	if (!n->data) {
		free(n);
		return NULL;
	}

	memcpy(n->data, data, size);
	n->size = size;
	n->next = NULL;

	return n;
}

int node_destroy(node *n) {
	if (!n) {
		return 1;
	}

	free(n->data);
	n->data = NULL;
	free(n);

	return 0;
} /* queue_c */
