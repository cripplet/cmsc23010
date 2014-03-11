#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "utils/hashgenerator.h"
#include "worker.h"

typedef struct dispatcher_t {
	HashPacketGenerator_t pks;
	int sources;
	worker **workers;
	float time;
	float folded_time;
	int uniform;
	long fingerprint;
	long packets;
	int M;
} dispatcher;

dispatcher *init_dispatcher(int, worker **, HashPacketGenerator_t, int, int M);

void *execute_dispatcher(void *);

#endif
