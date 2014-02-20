#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "result.h"
#include "serial.h"
#include "parallel.h"

#include "test.h"

#define COUNTERS 16
#define COUNTER_INCREMENT 100000
#define SLEEP_WAIT 1

struct test_lock_blob_t {
	volatile int counter;
	lock *counter_lock;
	volatile int is_done;
};

struct test_lock_blob_t *init_test_lock_blob(lock *l) {
	struct test_lock_blob_t *blob = malloc(sizeof(struct test_lock_blob_t));
	blob->counter = 0;
	blob->counter_lock = l;
	blob->is_done = 0;
	return(blob);
}

void *counter_increment(void *args) {
	struct test_lock_blob_t *b = (struct test_lock_blob_t *) args;

	void *slot = init_slot(b->counter_lock->type);

	for(volatile int i = 0; i < COUNTER_INCREMENT; i++) {
		l_lock(b->counter_lock, slot);
		b->counter++;
		l_unlock(b->counter_lock, slot);
	}

	l_lock(b->counter_lock, slot);
	b->is_done++;
	l_unlock(b->counter_lock, slot);

	pthread_exit(NULL);
	return(NULL);
}

int test_strategy(int strategy) {
	int t = 10000;
	int n = COUNTERS;
	long w = 8000;
	int uniform_flag = 1;
	short i = 1;

	result *p = serial_firewall(t, n, w, uniform_flag, i);
	result *q = parallel_firewall(t, n, w, uniform_flag, i, BACK, strategy);

	int success = (p->fingerprint == q->fingerprint);

	fprintf(stderr, "test_strategy(%i) results: %s (%li / %li)\n", strategy, success ? "MATCHED" : "DIDN'T MATCH", q->fingerprint, p->fingerprint);
	return(!success);
}

int test_lock(int type) {
	int lock_size = COUNTERS;

	lock *l = init_lock(type, &lock_size);

	struct test_lock_blob_t *b = init_test_lock_blob(l);

	pthread_t t;
	for(volatile int i = 0; i < COUNTERS; i++) {
		pthread_create(&t, NULL, counter_increment, b);
	}

	while(b->is_done != COUNTERS);

	int success = (b->counter == COUNTERS * COUNTER_INCREMENT);

	fprintf(stderr, "test_lock(%i) results: %s (%i / %i)\n", type, success ? "MATCHED" : "DIDN'T MATCH", b->counter, COUNTERS * COUNTER_INCREMENT);
	return(!success);
}
