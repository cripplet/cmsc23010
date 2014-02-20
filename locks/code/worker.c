#define _POSIX_C_SOURCE 199309L
#define SLEEP_TIME 4

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "utils/fingerprint.h"
#include "utils/stopwatch.h"
#include "queue.h"
#include "expr.h"

#include "worker.h"

worker *init_worker(int p_remaining, int q_size, int strategy) {
	worker *w = malloc(sizeof(worker));
	w->strategy = strategy;
	w->fingerprint = 0;
	w->is_done = 0;
	w->time = 0;
	w->p_remaining = p_remaining;
	w->queue = init_q(q_size);
	w->tspec.tv_nsec = SLEEP_TIME;
	return(w);
}

/**
 * Process the next packet on the queue.
 *
 * Spin-waits while the queue is empty.
 */
long process_packet(worker *w) {
	int aux = 0;

	// dequeue method
	Packet_t *pkt = NULL;
	switch(w->strategy) {
		case LFRE:
			pkt = deq(w->queue);
			break;
		case HOMQ:
			l_lock(w->queue->l, w->slot);
			pkt = deq(w->queue);
			l_unlock(w->queue->l, w->slot);
			break;
		case RNDQ:
			aux = rand() % w->num_peers;
			l_lock(w->peers[aux]->queue->l, w->slot);
			pkt = deq(w->peers[aux]->queue);
			l_unlock(w->peers[aux]->queue->l, w->slot);
			break;
		case LSTQ:
			while(!aux) {
				for(int i = 0; i < w->num_peers; i++) {
					if(!l_try(w->peers[i]->queue->l)) {
						aux = 1;
						l_lock(w->peers[i]->queue->l, w->slot);
						pkt = deq(w->peers[i]->queue);
						l_unlock(w->peers[i]->queue->l, w->slot);
					}
				}
			}
			break;
		case AWSM:
			break;
	}
	return(getFingerprint(pkt->iterations, pkt->seed));
}

/**
 * Main worker thread -- procesess w->p_remaining packets
 */
void *execute_worker(void *args) {
	// initial random seed
	srand(time(NULL));

	worker *w = args;

	// there is work to be done until there are no more incoming packets from the dispatcher (p_remaining)
	//	and the queue is empty
	int w_done = 0;
	while(w_done < w->num_peers) {
		w_done = 0;
		w->fingerprint += process_packet(w);
		for(int i = 0; i < w->num_peers; i++) {
			w_done += (!w->peers[i]->p_remaining && is_empty(w->peers[i]->queue));
		}
	}

	// signal to the dispatcher that this worker is done
	w->is_done = 1;

	pthread_exit(NULL);
	return(NULL);
}
