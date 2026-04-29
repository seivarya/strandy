#include "queue.h"
#include <stdlib.h>
#include <string.h>

int enqueue(queue *q, void *data, size_t size) {
	if (!q || !data || size == 0) return 1;

	node *n = malloc(sizeof(node));
	if (!n) return 1;

	n->data = malloc(size);
	if (!n->data) {
		free(n);
		return 1;
	}

	memcpy(n->data, data, size);
	n->size = size;
	n->next = NULL;

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

void *dequeue(queue *q) {
	if (!q || !q->head) return NULL;

	node *n = q->head;
	void *data = n->data;

	q->head = n->next;
	if (!q->head) q->tail = NULL;

	q->length--;

	free(n);   // IMPORTANT: free node only, not data

	return data;
}

int is_empty(queue *q) {
	return (q->length == 0);
}

queue* queue_create() {
	queue *q = malloc(sizeof(queue));
	if (!q) return NULL;

	q->head = NULL;
	q->tail = NULL;
	q->length = 0;

	return q;
}

int queue_destroy(queue *q) {
	if (!q) return 1;

	node *cur = q->head;
	while (cur) {
		node *next = cur->next;

		free(cur->data);   // free copied data
		free(cur);         // free node

		cur = next;
	}

	free(q);
	return 0;
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

