#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include "result.h"
#include "utils/packetsource.h"
#include "hash.h"
#include "type.h"

#include "test.h"

#define ATTEMPTS 100000
#define THREADS 8
#define LOG_THREADS 2
#define WORK 1000

typedef struct test_blob_t {
	hash_table *t;
	pthread_mutex_t lock;
	volatile int resource;
	volatile int is_done;
	packet_source *p;
} test_blob;

typedef struct thread_data_blob_t {
	test_blob *b;
	int start;
	int num;
} thread_data_blob;

thread_data_blob *thread_data_blob_init(int start, int num, test_blob *b) {
	thread_data_blob *tdb = malloc(sizeof(thread_data_blob));
	tdb->b = b;
	tdb->start = start;
	tdb->num = num;
	return(tdb);
}

test_blob *test_blob_init(hash_table *t, packet_source *p) {
	test_blob *b = malloc(sizeof(test_blob));
	pthread_mutex_init(&b->lock, NULL);
	b->t = t;
	b->resource = 0;
	b->is_done = 0;
	b->p = p;
	return(b);
}

int add_packets(hash_table *t, packet_source *p, int num_packets, int start);

int test_serial_add(hash_table *t, packet_source *p);
int test_serial_del(hash_table *t);
int test_serial_resize(hash_table *t, packet_source *p);
int test_serial_duplicate_add(hash_table *t, packet_source *p);
int test_parallel_duplicate_add(hash_table *t, packet_source *p);

void *parallel_add(void *args);

int test_hash(int type) {
	packet_source *p = createPacketSource(WORK, THREADS, 0);
	hash_table *t = ht_init(type, TABLE, LOG_THREADS);
	int result = (
		test_serial_add(t, p) &
		test_serial_duplicate_add(t, p) &
		test_serial_del(t) &
		test_serial_resize(t, p)
	);
	ht_free(t);

	t = ht_init(type, TABLE, LOG_THREADS);
	result &= test_parallel_duplicate_add(t, p);
	ht_free(t);

	deletePacketSource(p);
	return(result);
}

int test_serial_add(hash_table *t, packet_source *p) {
	add_packets(t, p, 1, 0);
	return(ht_contains(t, 0));
}
int test_serial_duplicate_add(hash_table *t, packet_source *p) {
	add_packets(t, p, 1, 0);
	return(!ht_add(t, 0, NULL));
}

int test_serial_del(hash_table *t) {
	ht_remove(t, 0);
	return(!ht_contains(t, 0));
}

int test_serial_resize(hash_table *t, packet_source *p) {
	int len = t->len;
	int key = add_packets(t, p,  t->len << 1, 0) - 1;
	return(ht_contains(t, key) & (t->len == len << 1));
}

int test_parallel_duplicate_add(hash_table *t, packet_source *p) {
	test_blob *b = test_blob_init(t, p);
	pthread_t tid;
	for(volatile int i = 0; i < THREADS; i++) {
		pthread_create(&tid, NULL, parallel_add, thread_data_blob_init(0, ATTEMPTS, b));
		pthread_detach(tid);
	}

	while(b->is_done < THREADS);
	int result = b->resource;

	free(b);
	printf("result is %i, of %i attempts, size %i\n", result, ATTEMPTS, t->size);
	return(result == ATTEMPTS);
}

/* Auxiliary functions */

void *parallel_add(void *args) {
	thread_data_blob *tdb = (thread_data_blob *) args;

	int added = add_packets(tdb->b->t, tdb->b->p, tdb->num, tdb->start);

	pthread_mutex_lock(&tdb->b->lock);
	tdb->b->resource += added;
	tdb->b->is_done += 1;
	pthread_mutex_unlock(&tdb->b->lock);

	free(tdb);
	pthread_exit(NULL);
	return(NULL);
}

/* Returns the number of packets added */
int add_packets(hash_table *t, packet_source *p, int num_packets, int start) {
	int added = 0;
	for(int i = 0; i < num_packets; i++) {
		packet *pkt = (packet *) getUniformPacket(p, 0);
		int result = ht_add(t, start + i, pkt);
		added += result;
		if(!result) {
			free(pkt);
		}
	}
	return(added);
}
