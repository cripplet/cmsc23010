#define _POSIX_C_SOURCE 199309L

#include <sys/time.h>
#include "hashpackettest.h"
#include "stopwatch.h"
#include "hashgenerator.h"
#include "paddedprim.h"
#include "statistics.h"
#include <stdbool.h>
#include <pthread.h>
#include "hashpacketworker.h"

#include <stdio.h>

#include "../result.h"
#include "../queue.h"
#include "../worker.h"
#include "../dispatcher.h"
#include "../expr.h"
#include "../type.h"
#include "../tune.h"
#include "../expr.h"
#include "../hash.h"

#include "../parallel.h"



static void millToTimeSpec(struct timespec *ts, unsigned long ms)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000;
}


result *serialHashPacketTest(int numMilliseconds, float fractionAdd, float fractionRemove, float hitRate, int maxBucketSize, long mean, int initSize) {
	StopWatch_t timer;

	PaddedPrimBool_NonVolatile_t done;
	done.value = false;

	// PaddedPrimBool_t memFence;
	// memFence.value = false;

	HashPacketGenerator_t * source = createHashPacketGenerator(fractionAdd,fractionRemove,hitRate,mean);
	SerialHashTable_t * table = createSerialHashTable(1, maxBucketSize);

	for( int i = 0; i < initSize; i++ ) {
	  HashPacket_t * pkt = getAddPacket(source);
	  add_ht(table,mangleKey(pkt), pkt->body);
	}

	pthread_t workerThread;
	int rc;
	pthread_attr_t attr;
	void *status;

	struct timespec tim;

	millToTimeSpec(&tim,numMilliseconds);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	SerialPacketWorker_t  workerData = {&done, source, table,0,0,0};

	rc = pthread_create(&workerThread, &attr, (void *) &serialWorker, (void *) &workerData);

	if (rc){
		fprintf(stderr,"ERROR; return code from pthread_create() for solo thread is %d\n", rc);
		exit(-1);
	}

	startTimer(&timer);

	nanosleep(&tim, NULL);

	done.value = true;

	// memFence.value = true;

	 rc = pthread_join(workerThread, &status);
	 if (rc) {
		 fprintf(stderr,"firewall error: return code for the threads using pthread_join() for solo thread  is %d\n", rc);
		 exit(-1);
	 }

	stopTimer(&timer);

	pthread_attr_destroy(&attr);

	long totalCount = workerData.totalPackets;

	result *r = init_result();
	r->packets = totalCount;
	r->time = getElapsedTime(&timer);
	return(r);
}

result *parallelHashPacketTest(int numMilliseconds, float fractionAdd, float fractionRemove, float hitRate, int maxBucketSize, long mean, int initSize, int numWorkers, int log_threads, int M, int H, int is_dropped)
{
	int numSources = numWorkers;
	StopWatch_t timer;


	int lock_type = MUTX;
	int strategy = LFRE;
	int uniformFlag = UNIFORM;

	hash_table *t = ht_init(H, TABLE, MAX_BUCKET_SIZE);

	worker **workers = malloc(numSources * sizeof(worker *));
	for(int i = 0; i < numSources; i++) {
		workers[i] = init_worker(0, Q_SIZE, strategy, t, is_dropped);
		workers[i]->slot = init_slot(lock_type);
		workers[i]->queue->l = init_lock(lock_type, &numSources);
	}

	// give workers a way to access peer queues
	for(int i = 0; i < numSources; i++) {
		workers[i]->peers = workers;
		workers[i]->num_peers = numSources;
	}

	// TODO -- fix this
	PacketSource_t *pks = createPacketSource(mean, numSources, 0);
	dispatcher *d = init_dispatcher(numSources, workers, *pks, uniformFlag, M);

	for(int i = 0; i < numSources; i++) {
		pthread_t tid;
		pthread_create(&tid, NULL, execute_worker, workers[i]);
	}

	struct timespec tim;

	millToTimeSpec(&tim,numMilliseconds);

	startTimer(&timer);

	pthread_t tid;
	pthread_create(&tid, NULL, execute_dispatcher, d);

	nanosleep(&tim , NULL);

	pthread_join(tid, NULL);

	stopTimer(&timer);

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
