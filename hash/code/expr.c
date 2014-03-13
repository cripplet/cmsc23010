#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/hashpackettest.h"

#include "counter.h"
#include "lock.h"
#include "tune.h"
#include "parallel.h"
#include "serial.h"

#include "type.h"
#include "expr.h"

#define TRIALS 1

void custom() {
	result *r;

	b = init_signal_blob(1);
	r = serialHashPacketTest(2000, .10, .10, .90, MAX_BUCKET_SIZE, MEAN_WORK, 0);
	fprintf(stderr, "S -- %li packets in %f milliseconds\n", r->packets, r->time);
	free(b);
	free(r);

	for(int i = 0; i < 3; i++) {
		int table_type = 1 << i;
		for(int drop_status = 0; drop_status < 2; drop_status++) {
			b = init_signal_blob(1);
			r = parallelHashPacketTest(2000, .10, .10, .90, MAX_BUCKET_SIZE, MEAN_WORK, 0, THREADS, LOG_THREADS, table_type, TWO_SECONDS, drop_status);

			fprintf(stderr, "P -- type == %i, drop == %i, %li packets in %f milliseconds\n", table_type, drop_status, r->packets, r->time);
			free(b);
			free(r);

			b = init_signal_blob(1);
			r = parallelHashPacketTest(2000, .08, .9, .01, MAX_BUCKET_SIZE, MEAN_WORK, 0, THREADS, LOG_THREADS, table_type, TWO_SECONDS, drop_status);
			fprintf(stderr, "P -- type == %i, drop == %i, %li packets in %f milliseconds\n", table_type, drop_status, r->packets, r->time);
			free(b);
			free(r);
		}
	}
}

// parallel no-load
void one() {
	fprintf(stderr, "executing exp one\n");
	result *r;
	for(int i = 0; i < TRIALS; i++) {
		b = init_signal_blob(1);
		r = parallelHashPacketTest(2000, .10, .10, .90, MAX_BUCKET_SIZE, 1, 0, THREADS, LOG_THREADS, LOCKING, TWO_SECONDS, 1);
		fprintf(stdout, "ONE (single): %f\n", r->packets / r->time * 1000);
		free(b);
		free(r);
	}
}

void two() {
	result *r;

	b = init_signal_blob(1);
	r = serialHashPacketTest(2000, .09, .01, .90, MAX_BUCKET_SIZE, MEAN_WORK, 0);
	fprintf(stdout, "TWO\tS\treads\t%f\n", r->packets / r->time * 1000);
	free(b);
	free(r);

	b = init_signal_blob(1);
	r = serialHashPacketTest(2000, .45, .05, .90, MAX_BUCKET_SIZE, MEAN_WORK, 0);
	fprintf(stdout, "TWO\tS\twrites\t%f\n", r->packets / r->time * 1000);
	free(b);
	free(r);

	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < TRIALS; j++) {
			int table_type = 1 << i;
			b = init_signal_blob(1);
			r = parallelHashPacketTest(2000, .09, .01, .90, MAX_BUCKET_SIZE, MEAN_WORK, 0, 1, 1, table_type, TWO_SECONDS, 0);
			fprintf(stdout, "TWO\tP\treads\t%f\t\%i\n", r->packets / r->time * 1000, table_type);
			free(b);
			free(r);

			b = init_signal_blob(1);
			r = parallelHashPacketTest(2000, .45, .05, .90, MAX_BUCKET_SIZE, MEAN_WORK, 0, 1, 1, table_type, TWO_SECONDS, 0);
			fprintf(stdout, "TWO\tP\twrites\t%f\t\%i\n", r->packets / r->time * 1000, table_type);
			free(b);
			free(r);
		}
	}
}

void three_aux(float hit_rate, int log_threads) {
	result *r;

	b = init_signal_blob(1);
	r = serialHashPacketTest(2000, .09, .01, hit_rate, MAX_BUCKET_SIZE, MEAN_WORK, 0);
	fprintf(stdout, "THREE\tS\treads\t%f\t%f\n", hit_rate, r->packets / r->time * 1000);
	free(b);
	free(r);

	b = init_signal_blob(1);
	r = serialHashPacketTest(2000, .45, .05, .90, MAX_BUCKET_SIZE, MEAN_WORK, 0);
	fprintf(stdout, "THREE\tS\twrites\t%f\t%f\n", hit_rate, r->packets / r->time * 1000);
	free(b);
	free(r);

	int num_threads = 1 << log_threads;

	for(int k = 0; k < 3; k++) {
		int table_type = 1 << k;
		for(int j = 0; j < TRIALS; j++) {
			b = init_signal_blob(1);
			r = parallelHashPacketTest(2000, .09, .01, hit_rate, MAX_BUCKET_SIZE, MEAN_WORK, 0, num_threads, log_threads, table_type, TWO_SECONDS, 0);
			fprintf(stdout, "THREE\t%i\treads\t%f\t%f\t\%i\n", num_threads, hit_rate, r->packets / r->time * 1000, table_type);
			free(b);
			free(r);

			b = init_signal_blob(1);
			r = parallelHashPacketTest(2000, .45, .05, hit_rate, MAX_BUCKET_SIZE, MEAN_WORK, 0, num_threads, log_threads, table_type, TWO_SECONDS, 0);
			fprintf(stdout, "THREE\t%i\twrites\t%f\t%f\t\%i\n", num_threads, hit_rate, r->packets / r->time * 1000, table_type);
			free(b);
			free(r);
		}
	}
}

void three(int log_threads) {
	three_aux(.5, log_threads);
	three_aux(.75, log_threads);
	three_aux(.9, log_threads);
	three_aux(.99, log_threads);
}
