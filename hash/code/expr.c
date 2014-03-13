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
		}
	}
}

// parallel no-load
void one() {
	fprintf(stderr, "executing exp one\n");
}

void two() {
	fprintf(stderr, "executing exp two\n");
}

void three() {
	fprintf(stderr, "executing exp three\n");
}
