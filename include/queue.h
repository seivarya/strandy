#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <pthread.h>

typedef struct node {
	void *data;
	struct node *next;
	size_t size;
} node;

typedef struct queue {
	node *head;
	node *tail; //  todo: not required for now i think?
	size_t length;

	pthread_mutex_t *lock;
	pthread_cond_t *cond;
} queue;


node* node_create(void *data, size_t size);
int node_destroy(node *n);

queue* queue_create();
int queue_destroy(queue *q);
void *peek(queue *q);


int enqueue(queue *q, void *data, size_t size);
node* dequeue(queue *q);
int is_empty(queue *q);

#endif /* queue_h */
