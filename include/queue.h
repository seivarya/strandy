#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

typedef struct node {
	void *data;
	struct node *next;
} node;

typedef struct queue {
	node *head;
	node *tail; //  todo: not required for now i think?
	size_t length;
} queue;


node* node_create(void *data, size_t size);
int node_destroy(node *n);

queue* queue_create();
int queue_destroy(queue *q);


int enqueue(queue *q, void *data, size_t size);
int dequeue(queue *q);
int is_empty(queue *q);
void* peek(queue *q);

#endif /* queue_h */
