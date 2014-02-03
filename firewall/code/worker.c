#include <pthread.h>

#include "utils/fingerprint.h"
#include "utils/stopwatch.h"
#include "queue.h"

#include "worker.h"

worker *init_worker(int p_remaining, int q_size) {
	worker *w = malloc(sizeof(worker));
	w->fingerprint = 0;
	w->is_done = 0;
	w->time = 0;
	w->p_remaining = p_remaining;
	w->queue = init_q(q_size);
	return(w);
}

/**
 * Process the next packet on the queue.
 *
 * Spin-waits while the queue is empty.
 */
long process_packet(worker *w) {
	while(is_empty(w->queue));
	Packet_t *pkt = deq(w->queue);
	return(getFingerprint(pkt->iterations, pkt->seed));
}

void *execute_worker(void *args) {
	StopWatch_t watch;
	startTimer(&watch);

	worker *w = args;
	while(w->p_remaining || !is_empty(w->queue)) {
		w->fingerprint += process_packet(w);
	}
	w->is_done = 1;

	stopTimer(&watch);
	w->time = getElapsedTime(&watch);

	pthread_exit(NULL);
	return(NULL);
}

void free_worker(worker *w) {
	free_q(w->queue);
	free(w);
}
