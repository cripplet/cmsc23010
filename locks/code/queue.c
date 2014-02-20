#include <stdlib.h>

#include "queue.h"

q *init_q(int size) {
	q *queue = malloc(sizeof(q));
	queue->size = size;
	queue->head = 0;
	queue->tail = 0;
	queue->elem = malloc(size * sizeof(Packet_t *));
	for(int i = 0; i < size; i++) {
		queue->elem[i] = malloc(sizeof(Packet_t));
	}
	return(queue);
}

int is_full(q *queue) {
	return((queue->tail - queue->head) == queue->size);
}

int is_empty(q *queue) {
	return(queue->tail == queue->head);
}

void enq(q *queue, Packet_t *pkt) {
	/**
	 * Only one thread is popping from the buffer.
	 */
	if(!is_full(queue)) {
		queue->elem[queue->tail % queue->size] = pkt;
		queue->tail++;
	}
}

Packet_t *deq(q *queue) {
	Packet_t *pkt = NULL;
	if(!is_empty(queue)) {
		pkt = queue->elem[queue->head % queue->size];
		queue->head++;
	}
	return(pkt);
}
