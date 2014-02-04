#ifndef WORKER_H
#define WORKER_H

#include "utils/stopwatch.h"

#include "queue.h"

typedef struct worker_t {
	long fingerprint;
	StopWatch_t watch;
	int p_remaining;
	float time;
	q *queue;
	int is_done;
} worker;

worker *init_worker(int, int);

void *execute_worker(void *);

long process_packet(worker *);
void free_worker(worker *);

#endif
