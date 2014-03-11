#define _POSIX_C_SOURCE 199309L

#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#include <stdio.h>

#include "tune.h"

#include "lock.h"

/* Intel i7-3770 chip has 64-byte L1 cache size, which is 16 ints per line */
#define CACHE_FACTOR 16

typedef struct lock_blob_t {
	volatile int *atomic_val;
	struct timespec *nsleep;

	// Anderson-specific lock struct
	int size;
	int *flags;

	// CLH-specifc lock struct
	volatile int * volatile *tail;
} lock_blob;

lock_blob *init_lock_blob() {
	lock_blob *b = malloc(sizeof(lock_blob));
	b->atomic_val = malloc(sizeof(int));
	b->tail = malloc(sizeof(int *));
	*b->tail = calloc(1, sizeof(int));
	return(b);
}

void *init_slot(int type) {
	void *slot = NULL;
	if(type == ALCK) {
		slot = malloc(sizeof(int));
	} else if(type == CLHQ) {
		slot = init_qnode();
	}
	return(slot);
}

void *init_ttas() {
	lock_blob *b = init_lock_blob();
	*(b->atomic_val) = 0;
	return(b);
}

void *init_back() {
	lock_blob *b = init_ttas();
	b->nsleep = calloc(1, sizeof(struct timespec));

	// iniitalize the backoff seed
	srand(time(NULL));
	return(b);
}

void *init_mutx() {
	pthread_mutex_t *mutx = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutx, NULL);
	return(mutx);
}

void *init_alck(void *args) {
	int *size = (int *) args;
	lock_blob *b = init_ttas();
	b->size = *size;
	b->flags = malloc(b->size * CACHE_FACTOR * sizeof(int));
	b->flags[0] = 1;
	return(b);
}

void lock_ttas(lock *l) {
	lock_blob *b = l->l;
	while(__sync_lock_test_and_set((b->atomic_val), 1)) {
		sched_yield();
	}
}

void lock_back(lock *l) {
	int limit = min_delay;
	int delay;

	lock_blob *b = l->l;
	b->nsleep->tv_nsec = min_delay;
	while(__sync_lock_test_and_set((b->atomic_val), 1)) {
		if(b->nsleep->tv_nsec < max_delay) {
			delay = rand() % limit;
			b->nsleep->tv_nsec = delay;
			limit = (max_delay > (limit << 2)) ? limit << 2 : max_delay;
		}
		nanosleep(b->nsleep, NULL);
	}
}

void lock_mutx(lock *l) {
	pthread_mutex_lock(l->l);
}

void lock_alck(lock *l, void *args) {
	lock_blob *b = l->l;

	int *slot = (int *) args;
	*slot = __sync_fetch_and_add(b->atomic_val, 1) % b->size;
	while(!b->flags[*slot * CACHE_FACTOR]) {
		sched_yield();
	}
}

void lock_clhq(lock *l, void *args) {
	lock_blob *b = l->l;

	qnode *q = (qnode *) args;

	*q->node = 1;
	q->pred = __sync_lock_test_and_set(b->tail, q->node);
	while(*q->pred) {
		sched_yield();
	}
}

void unlock_ttas(lock *l) {
	lock_blob *b = l->l;
	__sync_lock_release(b->atomic_val);
}

void unlock_mutx(lock *l) {
	pthread_mutex_unlock(l->l);
}

void unlock_alck(lock *l, void *args) {
	lock_blob *b = l->l;

	int *slot = (int *) args;

	b->flags[*slot * CACHE_FACTOR] = 0;
	b->flags[((*slot + 1) % b->size) * CACHE_FACTOR] = 1;
}

void unlock_clhq(lock *l, void *args) {
	qnode *q = (qnode *) args;

	*q->node = 0;
	q->node = q->pred;
}

qnode *init_qnode() {
	qnode *q = malloc(sizeof(qnode));
	q->node = calloc(1, sizeof(int));
	q->pred = NULL;
	return(q);
}

lock *init_lock(int type, void *args) {
	lock *l = malloc(sizeof(lock));
	l->type = type;
	l->status = 0;

	// create the lock type
	switch(type) {
		case TTAS:
		case CLHQ:
			l->l = init_ttas();
			break;
		case BACK:
			l->l = init_back();
			break;
		case MUTX:
			l->l = init_mutx();
			break;
		case ALCK:
			l->l = init_alck(args);
			break;
	}

	return(l);
}

int l_lock(lock *l, void *args) {
	switch(l->type) {
		case TTAS:
			lock_ttas(l);
			break;
		case BACK:
			lock_back(l);
			break;
		case MUTX:
			lock_mutx(l);
			break;
		case ALCK:
			lock_alck(l, args);
			break;
		case CLHQ:
			lock_clhq(l, args);
			break;
	}

	l->status = 1;
	return(0);
}

int l_try(lock *l) {
	return(l->status);
}

int l_unlock(lock *l, void *args) {
	l->status = 0;

	switch(l->type) {
		case TTAS:
		case BACK:
			unlock_ttas(l);
			break;
		case MUTX:
			unlock_mutx(l);
			break;
		case ALCK:
			unlock_alck(l, args);
			break;
		case CLHQ:
			unlock_clhq(l, args);
			break;
	}

	return(0);
}
