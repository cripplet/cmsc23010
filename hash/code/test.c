#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include "result.h"
#include "utils/packetsource.h"
#include "hash.h"
#include "type.h"

#include "test.h"

#define ATTEMPTS 10000
#define THREADS 8
#define LOG_THREADS 4
#define WORK 1000

typedef struct test_blob_t {
	hash_table *t;
	pthread_mutex_t lock;
	volatile int resource;
	volatile int wall;
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
	b->wall = 0;
	b->is_done = 0;
	b->p = p;
	return(b);
}

int add_packets(hash_table *t, packet_source *p, int num_packets, int start);

int test_serial_add(hash_table *t, packet_source *p);
int test_serial_del(hash_table *t);
int test_serial_resize(hash_table *t, packet_source *p);
int test_serial_duplicate_add(hash_table *t, packet_source *p);
int test_serial_duplicate_del(hash_table *t, packet_source *p);
int test_parallel_duplicate_add(hash_table *t, packet_source *p);
int test_parallel_duplicate_del(hash_table *t, packet_source *p);

void *parallel_add(void *args);
void *parallel_del(void *args);

int test_hash(int type) {
	int result = 1;
	packet_source *p = createPacketSource(WORK, THREADS, 0);

	hash_table *t;

	t = ht_init(type, TABLE, LOG_THREADS);
	result &= test_serial_add(t, p);
	result &= test_serial_del(t);
	ht_free(t);

	t = ht_init(type, TABLE, LOG_THREADS);
	result &= test_serial_resize(t, p);
	ht_free(t);

	t = ht_init(type, TABLE, LOG_THREADS);
	result &= test_serial_duplicate_add(t, p);
	ht_free(t);

	t = ht_init(type, TABLE, LOG_THREADS);
	result &= test_serial_duplicate_del(t, p);
	ht_free(t);

	t = ht_init(type, TABLE, LOG_THREADS);
	result &= test_parallel_duplicate_add(t, p);
	ht_free(t);

	t = ht_init(type, TABLE, LOG_THREADS);
	result &= test_parallel_duplicate_del(t, p);
	ht_free(t);

	deletePacketSource(p);
	return(result);
}

int test_serial_add(hash_table *t, packet_source *p) {
	add_packets(t, p, 1, 0);
	int success = ht_contains(t, 0);
	fprintf(stderr, "test_serial_add: %i\n", success);
	return(success);
}

int test_serial_duplicate_add(hash_table *t, packet_source *p) {
	int success = 1;
	add_packets(t, p, 1, 0);
	success &= !ht_add(t, 0, NULL);
	ht_remove(t, 0);
	success &= !ht_contains(t, 0);

	// add initial packets
	add_packets(t, p, ATTEMPTS, 0);
	success &= (t->size == ATTEMPTS);

	int offset = 10;

	// ensure that all duplicate keys are not added
	add_packets(t, p, ATTEMPTS, offset);
	success &= (t->size == ATTEMPTS + offset);

	for(int i = 0; i < ATTEMPTS + offset; i++) {
		success &= ht_contains(t, i);
	}

	fprintf(stderr, "test_serial_duplicate_add: %i\n", success);
	return(success);
}

int test_serial_duplicate_del(hash_table *t, packet_source *p) {
	int success = 1;
	add_packets(t, p, 1, 0);
	ht_remove(t, 0);
	success &= !ht_remove(t, 0);

	// add initial packets
	add_packets(t, p, ATTEMPTS, 0);
	success &= (t->size == ATTEMPTS);

	fprintf(stderr, "test_serial_duplicate_del: %i\n", success);
	return(success);
}

int test_serial_del(hash_table *t) {
	ht_remove(t, 0);
	int success = !ht_contains(t, 0);

	fprintf(stderr, "test_serial_del: %i\n", success);
	return(success);
}

int test_serial_resize(hash_table *t, packet_source *p) {
	int len = t->len;
	int key = add_packets(t, p, t->max_s << 1, 0) - 1;
	int success = ht_contains(t, key) & (t->len == len << 1);

	fprintf(stderr, "test_serial_resize: %i\n", success);
	return(success);
}

int test_parallel_duplicate_add(hash_table *t, packet_source *p) {
	test_blob *b = test_blob_init(t, p);
	pthread_t tid;
	for(volatile int i = 0; i < THREADS; i++) {
		pthread_create(&tid, NULL, parallel_add, thread_data_blob_init(0, ATTEMPTS, b));
		pthread_detach(tid);
	}

	while(b->is_done < THREADS) {
		sched_yield();
	}
	int result = b->resource;

	free(b);

	printf("result is %i, of %i expected; t->size = %i\n", result, ATTEMPTS, t->size);
	return(result == ATTEMPTS && result == t->size);
}

int test_parallel_duplicate_del(hash_table *t, packet_source *p) {
	test_blob *b = test_blob_init(t, p);
	pthread_t tid;
	for(volatile int i = 0; i < THREADS; i++) {
		pthread_create(&tid, NULL, parallel_del, thread_data_blob_init(0, ATTEMPTS, b));
		pthread_detach(tid);
	}

	while(b->is_done < THREADS) {
		sched_yield();
	}
	int result = b->resource;

	free(b);

	printf("result is %i, of %i expected; t->size = %i\n", result, ATTEMPTS, t->size);
	return(result == ATTEMPTS && t->size == 0);
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

void *parallel_del(void *args) {
	thread_data_blob *tdb = (thread_data_blob *) args;

	add_packets(tdb->b->t, tdb->b->p, tdb->num, tdb->start);

	pthread_mutex_lock(&tdb->b->lock);
	tdb->b->wall += 1;
	pthread_mutex_unlock(&tdb->b->lock);

	while(tdb->b->wall != THREADS) {
		sched_yield();
	}

	int deleted = 0;
	for(int i = tdb->start; i < tdb->num; i++) {
		deleted += ht_remove(tdb->b->t, tdb->start + i);
	}

	pthread_mutex_lock(&tdb->b->lock);
	tdb->b->resource += deleted;
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
