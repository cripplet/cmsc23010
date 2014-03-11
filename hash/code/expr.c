#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "counter.h"
#include "lock.h"
#include "tune.h"
#include "parallel.h"
#include "serial.h"

#include "type.h"
#include "expr.h"

#define THREADS 4
#define LOG_THREADS 2
#define MEAN_WORK 1000
#define TWO_SECONDS 2

void custom() {
	b = init_signal_blob(1);
	result *r = serial_firewall(LOG_THREADS, THREADS, MEAN_WORK, 0, TWO_SECONDS);
	fprintf(stderr, "serially hashed %li packets in %i seconds\n", r->packets, TWO_SECONDS);
	free(b);
	free(r);

	int drop = 1;

	b = init_signal_blob(1);
	r = parallel_firewall(LOG_THREADS, THREADS, MEAN_WORK, 0, TWO_SECONDS, LOCKING, !drop);
	fprintf(stderr, "parallel hashed %li packets in %i seconds (thread == %i)\n", r->packets, TWO_SECONDS, THREADS);
	free(b);
	free(r);

	b = init_signal_blob(1);
	r = parallel_firewall(LOG_THREADS, THREADS, MEAN_WORK, 0, TWO_SECONDS, LOCKING, drop);
	fprintf(stderr, "parallel hashed %li packets in %i seconds (thread == %i) (dropped)\n", r->packets, TWO_SECONDS, THREADS);
	free(b);
	free(r);
}
