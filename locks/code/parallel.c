#include <pthread.h>

#include <stdio.h>

#include "queue.h"
#include "worker.h"
#include "dispatcher.h"

#include "parallel.h"

result *parallel_firewall(int numPackets, int numSources, long mean, int uniformFlag, short experimentNumber, int lock_type, int strategy) {
	worker **workers = malloc(numSources * sizeof(worker *));
	for(int i = 0; i < numSources; i++) {
		workers[i] = init_worker(numPackets, Q_SIZE, strategy);
		workers[i]->slot = init_slot(lock_type);
		workers[i]->queue->l = init_lock(lock_type, &numSources);
	}

	// give workers a way to access peer queues
	for(int i = 0; i < numSources; i++) {
		workers[i]->peers = workers;
		workers[i]->num_peers = numSources;
	}

	PacketSource_t *pks = createPacketSource(mean, numSources, experimentNumber);
	dispatcher *d = init_dispatcher(numSources, workers, *pks, uniformFlag);

	for(int i = 0; i < numSources; i++) {
		pthread_t tid;
		pthread_create(&tid, NULL, execute_worker, workers[i]);
	}

	pthread_t tid;
	pthread_create(&tid, NULL, execute_dispatcher, d);
	pthread_join(tid, NULL);

	result *r = init_result();
	r->time = d->time;

	// what we actually want to analyze
	r->folded_time = d->folded_time;

	r->fingerprint = d->fingerprint;

	return(r);
}
