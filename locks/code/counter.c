#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>

#include "utils/stopwatch.h"
#include "lock.h"
#include "expr.h"

#include "counter.h"

typedef struct signal_blob_t {
	volatile int flags;
} signal_blob;

typedef struct counter_t {
	volatile int i;
	int type;
	int bound;
	lock *l;
} counter;

signal_blob *b;

void ALARM_handler_counter(int sig) {
	b->flags--;
}

signal_blob *init_signal_blob(int flags) {
	signal_blob *b = malloc(sizeof(signal_blob));
	b->flags = flags;
	return(b);
}

counter *init_counter() {
	counter *c = malloc(sizeof(counter));
	c->i = 0;
	return(c);
}

int time_counter_serial(int M) {
	signal(SIGALRM, ALARM_handler_counter);

	b = init_signal_blob(1);

	counter *c = init_counter();
	ualarm((useconds_t) M, 0);

	while(b->flags) {
		c->i++;
	}

	return(c->i);
}

void *time_counter_handler(void *args) {
	counter *c = (counter *) args;

	int to_log = (counter_5_result != NULL);

	void *slot = init_slot(c->l->type);

	while(b->flags) {
		l_lock(c->l, slot);
		c->i++;
		if(to_log) {
			counter_5_result->contributions[pthread_self() % counter_5_result->n]++;
		}
		l_unlock(c->l, slot);
	}

	pthread_exit(NULL);
	return(NULL);
}

int time_counter_parallel(int M, int n, int L) {
	signal(SIGALRM, ALARM_handler_counter);

	b = init_signal_blob(1);

	counter *c = init_counter();
	c->l = init_lock(L, &n);

	pthread_t t;

	ualarm((useconds_t) M, 0);
	for(volatile int i = 0; i < n; i++) {
		pthread_create(&t, NULL, time_counter_handler, c);
	}

	while(b->flags) {
		sched_yield();
	}

	return(c->i);
}

float work_counter_serial(int B) {
	counter *c = init_counter();
	c->bound = B;

	StopWatch_t watch;
	startTimer(&watch);
	while(c->i < c->bound) {
		c->i++;
	}
	stopTimer(&watch);
	return(getElapsedTime(&watch));
}

void *work_counter_handler(void *args) {
	counter *c = (counter *) args;

	void *slot = init_slot(c->l->type);

	while(c->i < c->bound) {
		l_lock(c->l, slot);
		c->i++;
		l_unlock(c->l, slot);
	}

	pthread_exit(NULL);
	return(NULL);
}

float work_counter_parallel(int B, int n, int L) {
	counter *c = init_counter();
	c->bound = B;
	c->l = init_lock(L, &n);

	pthread_t t;

	StopWatch_t watch;
	startTimer(&watch);
	for(volatile int i = 0; i < n; i++) {
		pthread_create(&t, NULL, work_counter_handler, c);
	}

	while(c->i < c->bound) {
		sched_yield();
	}

	stopTimer(&watch);
	return(getElapsedTime(&watch));
}

