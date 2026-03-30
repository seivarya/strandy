#include "queue.h"

#include <stdlib.h>
#include <string.h>

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
	n->next = NULL;

	return n;
}

int node_destroy(node *n) {
	if (!n) {
		return 1;
	}

	free(n->data);
	free(n);
	return 0;
}

queue* queue_create() {
	queue *q = malloc(sizeof(queue));
	if (!q) {
		return NULL;
	}

	q->head = NULL;
	q->tail = NULL;
	q->length = 0;

	return q;
}

int queue_destroy(queue *q) {
	if (!q) {
		return 1;
	}

	node *cur = q->head;
	while (cur) {
		node *next = cur->next;
		node_destroy(cur);
		cur = next;
	}

	free(q);
	return 0;
}

int enqueue(queue *q, void *data, size_t size) {
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
	return 0;
}

int dequeue(queue *q) {
	if (!q || !q->head) {
		return 1;
	}

	node *n = q->head;
	q->head = q->head->next;

	if (!q->head) {
		q->tail = NULL;
	}

	node_destroy(n);
	q->length--;
	return 0;
}

int is_empty(queue *q) {
	if (!q) {
		return 1;
	}

	return (q->length == 0);
}

void* peek(queue *q) {
	if (!q || !q->head) {
		return NULL;
	}

	return q->head->data;
} /* queue_c */
