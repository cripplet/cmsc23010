#define _POSIX_C_SOURCE 199309L

#include <pthread.h>
#include <unistd.h>

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
	w->tspec.tv_nsec = 4;
	return(w);
}

/**
 * Process the next packet on the queue.
 *
 * Spin-waits while the queue is empty.
 */
long process_packet(worker *w) {
	startTimer(&w->watch);
	while(is_empty(w->queue)) {
		nanosleep(&w->tspec, NULL);
	}
	stopTimer(&w->watch);

	w->time += getElapsedTime(&w->watch);

	Packet_t *pkt = deq(w->queue);
	return(getFingerprint(pkt->iterations, pkt->seed));
}

/**
 * Main worker thread -- procesess w->p_remaining packets
 */
void *execute_worker(void *args) {
	worker *w = args;

	// there is work to be done until there are no more incoming packets from the dispatcher (p_remaining)
	//	and the queue is empty
	while(w->p_remaining || !is_empty(w->queue)) {
		w->fingerprint += process_packet(w);
	}

	// signal to the dispatcher that this worker is done
	w->is_done = 1;

	pthread_exit(NULL);
	return(NULL);
}

void free_worker(worker *w) {
	free_q(w->queue);
	free(w);
}
