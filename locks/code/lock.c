#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#include "tune.h"

#include "lock.h"

typedef struct lock_blob_t {
	volatile int *atomic_val;
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
	// iniitalize the backoff seed
	srand(time(NULL));
	return(b);
}

void *init_mutx() {
	pthread_mutex_t *mutx = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutx, NULL);
	return(mutx);
}

void lock_ttas(lock *l) {
	lock_blob *b = l->l;
	while(__sync_lock_test_and_set((volatile int *) (b->atomic_val), 1));
}

void lock_back(lock *l) {
	lock_blob *b = l->l;
	
	while(__sync_lock_test_and_set((volatile int *) (b->atomic_val), 1)) {
	}
}

void lock_mutx(lock *l) {
	pthread_mutex_lock(l->l);
}

void unlock_ttas(lock *l) {
	lock_blob *b = l->l;
	__sync_lock_release((volatile int *) (b->atomic_val));
}

void unlock_mutx(lock *l) {
	pthread_mutex_unlock(l->l);
}

lock *init_lock(int type) {
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
			break;
		case CLHQ:
			break;
		case MCSQ:
			break;
	}

	return(l);
}

int l_lock(lock *l) {
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
	}

	l->status = 1;
	return(1);
}

int l_try(lock *l) {
	return(l->status);
}

int l_unlock(lock *l) {
	l->status = 0;

	switch(l->type) {
		case TTAS:
		case BACK:
			unlock_ttas(l);
			break;
		case MUTX:
			unlock_mutx(l);
			break;
	}

	return(1);
}
