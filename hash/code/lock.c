#include <stdlib.h>

#include "lock.h"

/**
 * Simple read-write lock
 *
 * cf. Herlihy section 8.3
 */

read_write_lock *read_write_lock_init() {
	read_write_lock *l = malloc(sizeof(read_write_lock));
	l->readers = 0;
	l->writer = 0;
	pthread_cond_init(&l->condition, NULL);
	pthread_mutex_init(&l->lock, NULL);
	pthread_mutex_init(&l->r_lock, NULL);
	pthread_mutex_init(&l->w_lock, NULL);
	return(l);
}

void read_write_lock_r_lock(read_write_lock *l) {
	pthread_mutex_lock(&l->lock);
	while(l->writer) {
		pthread_cond_wait(&l->condition, &l->lock);
	}
	l->readers++;
	pthread_mutex_unlock(&l->lock);
}

void read_write_lock_w_lock(read_write_lock *l) {
	pthread_mutex_lock(&l->lock);
	while(l->readers) {
		pthread_cond_wait(&l->condition, &l->lock);
	}
	l->writer = 1;
	pthread_mutex_unlock(&l->lock);
}

void read_write_lock_r_unlock(read_write_lock *l) {
	pthread_mutex_lock(&l->lock);
	l->readers--;
	if(!l->readers) {
		pthread_cond_broadcast(&l->condition);
	}
	pthread_mutex_unlock(&l->lock);
}

void read_write_lock_w_unlock(read_write_lock *l) {
	l->writer = 0;
	pthread_cond_broadcast(&l->condition);
}

void read_write_lock_free(read_write_lock *l) {
	free(l);
}
