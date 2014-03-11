#ifndef WORKER_H
#define WORKER_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <sys/time.h>

#include "utils/stopwatch.h"

#include "hash.h"

#include "queue.h"

typedef struct worker_t {
	int strategy;
	struct worker_t **peers;
	int num_peers;

	void *slot;

	long fingerprint;
	StopWatch_t watch;
	int p_remaining;
	float time;
	q *queue;
	int is_done;
	struct timespec tspec;

	hash_table *t;
	long packets;
	int is_dropped;
} worker;

worker *init_worker(int, int, int, hash_table *t, int is_dropped);

void *execute_worker(void *);

long process_packet(worker *);
void free_worker(worker *);

#endif
