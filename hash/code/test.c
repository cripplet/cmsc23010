#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include "result.h"
#include "utils/packetsource.h"
#include "hash.h"
#include "type.h"

#include "test.h"

#define THREADS 16
#define WORK 1000

int test_single_add(hash_table *t, packet_source *p);
int test_single_del(hash_table *t);

int test_hash(int type) {
	packet_source *p = createPacketSource(WORK, THREADS, 0);
	hash_table *t = ht_init(type, TABLE, THREADS);
	int result = (
		test_single_add(t, p) &
		test_single_del(t)
	);
	ht_free(t);
	deletePacketSource(p);
	return(result);
}

int test_single_add(hash_table *t, packet_source *p) {
	packet *pkt = (packet *) getUniformPacket(p, 0);
	ht_add(t, 0, pkt);
	return(ht_contains(t, 0));
}
int test_single_del(hash_table *t) {
	ht_remove(t, 0);
	return(!ht_contains(t, 0));
}

/**

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
*/
