#ifndef WORKER_H
#define WORKER_H

#define _POSIX_C_SOURCE 199309L
#include <sys/time.h>

#include "utils/stopwatch.h"

#include "queue.h"

typedef struct worker_t {
	long fingerprint;
	StopWatch_t watch;
	int p_remaining;
	float time;
	q *queue;
	int is_done;
	struct timespec tspec;
} worker;

worker *init_worker(int, int);

void *execute_worker(void *);

long process_packet(worker *);
void free_worker(worker *);

#endif
