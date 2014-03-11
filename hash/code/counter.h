#ifndef COUNTER_H
#define COUNTER_H
#include "tune.h"
#include "counter.h"

typedef struct signal_blob_t {
	volatile int flags;
} signal_blob;

signal_blob *b;

void ALARM_handler_counter(int sig);
signal_blob *init_signal_blob(int flags);

#endif
