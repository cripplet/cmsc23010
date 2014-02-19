#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"

#define COUNTERS 10
#define COUNTER_INCREMENT 1000
#define SLEEP_WAIT 1

struct test_lock_blob_t {
	int counter;
	lock *counter_lock;
};

struct test_lock_blob_t *init_test_lock_blob(lock *l) {
	struct test_lock_blob_t *blob = malloc(sizeof(struct test_lock_blob_t));
	blob->counter = 0;
	blob->counter_lock = l;
	return(blob);
}

void *counter_increment(void *args) {
	struct test_lock_blob_t *b = (struct test_lock_blob_t *) args;

	int slot;

	for(int i = 0; i < COUNTER_INCREMENT; i++) {
		l_lock(b->counter_lock->l, &slot);
		b->counter++;
		l_unlock(b->counter_lock->l, &slot);
	}

	pthread_exit(NULL);
	return(NULL);
}

int test_lock(int type) {
	int lock_size = COUNTERS;

	lock *l = init_lock(type, &lock_size);

	struct test_lock_blob_t *b = init_test_lock_blob(l);

	pthread_t t;
	for(int i = 0; i < COUNTERS; i++) {
		pthread_create(&t, NULL, counter_increment, b);
	}

	sleep(SLEEP_WAIT);	// give time to increment

	int result = (b->counter == COUNTERS * COUNTER_INCREMENT);

	fprintf(stderr, "test_lock(%i) results: %s\n", type, result ? "MATCHED" : "DIDN'T MATCH");

	return(!result);
}
