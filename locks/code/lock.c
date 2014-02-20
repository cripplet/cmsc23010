#define _POSIX_C_SOURCE 199309L

#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#include <stdio.h>

#include "tune.h"

#include "lock.h"

typedef struct lock_blob_t {
	volatile int *atomic_val;
	struct timespec *nsleep;

	// Anderson-specific lock struct
	int size;
	int *flags;
} lock_blob;

lock_blob *init_lock_blob() {
	lock_blob *b = malloc(sizeof(lock_blob));
	b->atomic_val = malloc(sizeof(int));
	return(b);
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
	b->flags = malloc(b->size * sizeof(int));
	b->flags[0] = 1;
	return(b);
}

void lock_ttas(lock *l) {
	lock_blob *b = l->l;
	while(__sync_lock_test_and_set((volatile int *) (b->atomic_val), 1)) {
		sched_yield();
	}
}

void lock_back(lock *l) {
	int limit = MIN_DELAY;
	int delay;

	lock_blob *b = l->l;
	b->nsleep->tv_nsec = MIN_DELAY;
	while(__sync_lock_test_and_set((volatile int *) (b->atomic_val), 1)) {
		if(b->nsleep->tv_nsec < MAX_DELAY) {
			delay = rand() % limit;
			b->nsleep->tv_nsec = (delay < limit) ? delay : limit;
			limit <<= 2;
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
	*slot = __sync_fetch_and_add((volatile int *) (b->atomic_val), 1) % b->size;
	while(!b->flags[*slot]) {
		sched_yield();
	}
}

void unlock_ttas(lock *l) {
	lock_blob *b = l->l;
	__sync_lock_release((volatile int *) (b->atomic_val));
}

void unlock_mutx(lock *l) {
	pthread_mutex_unlock(l->l);
}

void unlock_alck(lock *l, void *args) {
	lock_blob *b = l->l;

	int *slot = (int *) args;

	b->flags[*slot] = 0;
	b->flags[(*slot + 1) % b->size] = 1;
}

lock *init_lock(int type, void *args) {
	lock *l = malloc(sizeof(lock));
	l->type = type;
	l->status = 0;

	// create the lock type
	switch(type) {
		case TTAS:
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
		case CLHQ:
			break;
		case MCSQ:
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
	}

	return(0);
}
