#ifndef QUEUE_H
#define QUEUE_H

#include "lock.h"

#include "utils/hashgenerator.h"
#include "utils/packetsource.h"

#define Q_SIZE 32

typedef struct q_t {
	int size;
	int head;
	int tail;
	HashPacket_t **elem;
	lock *l;		// an associated lock
} q;

q *init_q(int);
void free_q(q *);

void enq(q *, HashPacket_t *);
HashPacket_t *deq(q *);

int is_full(q *);
int is_empty(q *);

#endif
