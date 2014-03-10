#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>

typedef struct read_write_lock_t {
	int readers;
	int writer;
	pthread_cond_t condition;
	pthread_mutex_t lock;
	pthread_mutex_t r_lock;
	pthread_mutex_t w_lock;
} read_write_lock;

/* Memory allocation */
read_write_lock *read_write_lock_init();
void read_write_lock_free(read_write_lock *l);

/* Locking schemas */

/* Read-write lock */
void read_write_lock_r_lock(read_write_lock *l);
void read_write_lock_w_lock(read_write_lock *l);
void read_write_lock_r_unlock(read_write_lock *l);
void read_write_lock_w_unlock(read_write_lock *l);

#endif
