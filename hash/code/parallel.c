#include <pthread.h>

#include <stdio.h>

#include "queue.h"
#include "worker.h"
#include "dispatcher.h"
#include "expr.h"
#include "type.h"
#include "tune.h"
#include "hash.h"

#include "parallel.h"

result *parallel_firewall(int log_threads, int numSources, long mean, short experimentNumber, int M, int H) {
	int lock_type = MUTX;
	int strategy = LFRE;
	int uniformFlag = UNIFORM;

	hash_table *t = ht_init(H, TABLE, MAX_BUCKET_SIZE);

	worker **workers = malloc(numSources * sizeof(worker *));
	for(int i = 0; i < numSources; i++) {
		workers[i] = init_worker(0, Q_SIZE, strategy, t);
		workers[i]->slot = init_slot(lock_type);
		workers[i]->queue->l = init_lock(lock_type, &numSources);
	}

	// give workers a way to access peer queues
	for(int i = 0; i < numSources; i++) {
		workers[i]->peers = workers;
		workers[i]->num_peers = numSources;
	}

	PacketSource_t *pks = createPacketSource(mean, numSources, experimentNumber);
	dispatcher *d = init_dispatcher(numSources, workers, *pks, uniformFlag, M);

	for(int i = 0; i < numSources; i++) {
		pthread_t tid;
		pthread_create(&tid, NULL, execute_worker, workers[i]);
	}

	pthread_t tid;
	pthread_create(&tid, NULL, execute_dispatcher, d);
	pthread_join(tid, NULL);

	result *r = init_result();
	// what we actually want to analyze
	r->packets = d->packets;
	r->fingerprint = d->fingerprint;

	for(int i = 0; i < numSources; i++) {
		free(workers[i]->slot);
		free(workers[i]->queue->l);
		free(workers[i]->queue->elem);
		free(workers[i]->queue);
		free(workers[i]);
	}
	free(workers);

	return(r);
}
