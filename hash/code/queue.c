#include <stdlib.h>

#include "queue.h"

q *init_q(int size) {
	q *queue = malloc(sizeof(q));
	queue->size = size;
	queue->head = 0;
	queue->tail = 0;
	queue->elem = malloc(size * sizeof(HashPacket_t *));
	for(int i = 0; i < size; i++) {
		queue->elem[i] = malloc(sizeof(HashPacket_t));
	}
	return(queue);
}

int is_full(q *queue) {
	return((queue->tail - queue->head) == queue->size);
}

int is_empty(q *queue) {
	return(queue->tail == queue->head);
}

void enq(q *queue, HashPacket_t *pkt) {
	/**
	 * Only one thread is popping from the buffer.
	 */
	if(!is_full(queue)) {
		queue->elem[queue->tail % queue->size] = pkt;
		queue->tail++;
	}
}

HashPacket_t *deq(q *queue) {
	HashPacket_t *pkt = NULL;
	if(!is_empty(queue)) {
		pkt = queue->elem[queue->head % queue->size];
		queue->head++;
	}
	return(pkt);
}
