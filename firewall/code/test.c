#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "utils/packetsource.h"
#include "utils/generators.h"
#include "utils/fingerprint.h"

#include "queue.h"

#define DIM 16
#define TEST_Q_SIZE 6

typedef struct test_queue_blob {
	q *queue;
	Packet_t *packets[DIM];
} tq_blob;

tq_blob *init_tq_blob() {
	PacketSource_t *ps = createPacketSource(100, DIM, 0);

	tq_blob *blob = malloc(sizeof(tq_blob));
	blob->queue = init_q(TEST_Q_SIZE);
	for(int i = 0; i < DIM; i++) {
		blob->packets[i] = (Packet_t *) getUniformPacket(ps, 1);
	}

	free(ps);
	return(blob);
}

/**
 * test prototypes
 */
int test_enq();
void *enq_thread(void *);
void *enq_thread_sleep(void *);

int test_all() {
	return(
		test_enq(enq_thread) &
		test_enq(enq_thread_sleep)
	);
}

int test_enq(void * (*func) (void *)) {
	int success = 1;

	tq_blob *blob = init_tq_blob();

	pthread_t t;
	pthread_create(&t, NULL, func, blob);

	int i = 0;
	while(i < DIM) {
		if(!is_empty(blob->queue)) {
			Packet_t *pkt = deq(blob->queue);
			success = (getFingerprint(pkt->iterations, pkt->seed) == getFingerprint(blob->packets[i]->iterations, blob->packets[i]->seed));
			i++;
		}
	}

	fprintf(stderr, "exiting test_enq with success == %i\n", success);
	return(success);
}

void *enq_thread(void *args) {
	tq_blob *blob = args;

	int i = 0;
	while(i < DIM) {
		if(!is_full(blob->queue)) {
			enq(blob->queue, blob->packets[i]);
			i++;
		}
	}

	pthread_exit(NULL);
}

void *enq_thread_sleep(void *args) {
	tq_blob *blob = args;

	int i = 0;
	while(i < DIM) {
		if(!is_full(blob->queue)) {
			enq(blob->queue, blob->packets[i]);
			i++;
			sleep(1);
		}
	}

	pthread_exit(NULL);
}
