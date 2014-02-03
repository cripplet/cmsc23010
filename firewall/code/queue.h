#ifndef QUEUE_H
#define QUEUE_H

#include "utils/packetsource.h"

typedef struct q_t {
	int size;
	int head;
	int tail;
	Packet_t **elem;
} q;

q *init_q(int);
void free_q(q *);

void enq(q *, Packet_t *);
Packet_t *deq(q *);

int is_full(q *);
int is_empty(q *);

#endif
