#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "worker.h"

typedef struct dispatcher_t {
	PacketSource_t pks;
	int sources;
	worker **workers;
	float time;
	float folded_time;
	int uniform;
	long fingerprint;
} dispatcher;

dispatcher *init_dispatcher(int, worker **, PacketSource_t, int);

void *execute_dispatcher(void *);

#endif
