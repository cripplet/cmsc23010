#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#define SLEEP_TIME 4

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "utils/hashgenerator.h"
#include "utils/fingerprint.h"
#include "utils/stopwatch.h"
#include "queue.h"
#include "expr.h"
#include "counter.h"

#include "worker.h"

worker *init_worker(int p_remaining, int q_size, int strategy, hash_table *t, int is_dropped) {
	worker *w = malloc(sizeof(worker));
	w->strategy = strategy;
	w->fingerprint = 0;
	w->is_done = 0;
	w->time = 0;
	w->p_remaining = p_remaining;
	w->queue = init_q(q_size);
	w->tspec.tv_nsec = SLEEP_TIME;
	w->tspec.tv_sec = 0;
	w->packets = 0;
	w->t = t;
	w->is_dropped = is_dropped;
	return(w);
}

void hash_pkt(HashPacket_t *p, hash_table *t, int is_dropped) {
	if(is_dropped) {
		return;
	}
	switch(p->type) {
	case Add:
		ht_add(t, mangleKey(p), (packet *) p->body);
		break;
	case Remove:
		ht_remove(t, mangleKey(p));
		break;
	case Contains:
		ht_contains(t, mangleKey(p));
		break;
	}
}

/**
 * Process the next packet on the queue.
 *
 * Spin-waits while the queue is empty.
 */
long process_packet(worker *w) {
	int aux = 0;

	// dequeue method
	HashPacket_t *pkt = NULL;
	switch(w->strategy) {
		case LFRE:
			while(is_empty(w->queue)) {
				nanosleep(&w->tspec, NULL);
			}
			pkt = deq(w->queue);
			break;
		case HOMQ:
			while(is_empty(w->queue)) {
				nanosleep(&w->tspec, NULL);
			}
			l_lock(w->queue->l, w->slot);
			pkt = deq(w->queue);
			l_unlock(w->queue->l, w->slot);
			break;
		case RNDQ:
			aux = rand() % w->num_peers;
			l_lock(w->peers[aux]->queue->l, w->slot);
			if(is_empty(w->peers[aux]->queue)) {
				l_unlock(w->peers[aux]->queue->l, w->slot);
				nanosleep(&w->tspec, NULL);
				return(0);
			}
			pkt = deq(w->peers[aux]->queue);
			l_unlock(w->peers[aux]->queue->l, w->slot);
			break;
		case LSTQ:
			for(int tries = 0; tries < 5; tries++) {
				aux = rand() % w->num_peers;
				if(!l_try(w->peers[aux]->queue->l)) {
					l_lock(w->peers[aux]->queue->l, w->slot);
					long fingerprint = 0;
					// delay a while just in case there are more packets
					if(w->peers[aux]->p_remaining) {
						nanosleep(&w->tspec, NULL);
					}
					while(!is_empty(w->peers[aux]->queue)) {
						pkt = deq(w->peers[aux]->queue);
						fingerprint += getFingerprint(pkt->body->iterations, pkt->body->seed);
						hash_pkt(pkt, w->t, w->is_dropped);
					}
					l_unlock(w->peers[aux]->queue->l, w->slot);
					return(fingerprint);
				}
			}
			nanosleep(&w->tspec, NULL);
			return(0);
		// do work on own queue until the queue is empty --
		//	then search in an orderly manner for empty queue
		case AWSM:
			l_lock(w->queue->l, w->slot);
			for(int tries = 0; tries < 3; tries++) {
				if(is_empty(w->queue)) {
					nanosleep(&w->tspec, NULL);
				} else {
					pkt = deq(w->queue);
					break;
				}
			}
			l_unlock(w->queue->l, w->slot);
			if(pkt == NULL) {
				for(int tries = 0; tries < 1; tries++) {
					aux = rand() % w->num_peers;
					l_lock(w->peers[aux]->queue->l, w->slot);
					if(is_empty(w->peers[aux]->queue)) {
						nanosleep(&w->tspec, NULL);
					} else {
						pkt = deq(w->peers[aux]->queue);
						l_unlock(w->peers[aux]->queue->l, w->slot);
						break;
					}
					l_unlock(w->peers[aux]->queue->l, w->slot);
				}
			}
			if(pkt == NULL) {
				return(0);
			}
	}
	if(pkt == NULL) {
		return(0);
	}
	int fingerprint = getFingerprint(pkt->body->iterations, pkt->body->seed);
	hash_pkt(pkt, w->t, w->is_dropped);
	return(fingerprint);
}

/**
 * Main worker thread -- procesess w->p_remaining packets
 */
void *execute_worker(void *args) {
	worker *w = args;

	// there is work to be done until there are no more incoming packets from the dispatcher (p_remaining)
	//	and the queue is empty
	while(b->flags) {
		if(!is_empty(w->queue)) {
			long f = process_packet(w);
			w->fingerprint += f;
			w->p_remaining += 1;
		}
	}

	// signal to the dispatcher that this worker is done
	w->is_done = 1;

	pthread_exit(NULL);
	return(NULL);
}
