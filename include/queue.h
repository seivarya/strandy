#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

typedef struct node {
	void *data;
	struct node *next;
	size_t size;
} node;

typedef struct queue {
	node *head;
	node *tail;
	size_t length;

	int shutdown;
} queue;


node* node_create(void *data, size_t size);
int node_destroy(node *n);

queue* queue_create(void);
int queue_destroy(queue *q);


int enqueue(queue *q, void *data, size_t size);
void* dequeue(queue *q);
int is_empty(queue *q);

#endif /* queue_h */
