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

void ALARM_handler_counter(int sig) {
	b->flags--;
}

signal_blob *init_signal_blob(int flags) {
	signal_blob *b = malloc(sizeof(signal_blob));
	b->flags = flags;
	return(b);
}
