#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "utils/hashgenerator.h"
#include "utils/stopwatch.h"

#include "counter.h"

#include "dispatcher.h"

dispatcher *init_dispatcher(int sources, worker **workers, HashPacketGenerator_t pks, int uniform_flag, int M) {
	dispatcher *d = malloc(sizeof(dispatcher));
	d->pks = pks;
	d->sources = sources;
	d->workers = workers;
	d->time = 0;
	d->folded_time = 0;
	d->uniform = uniform_flag;
	d->fingerprint = 0;
	d->packets = 0;
	d->M = M;
	return(d);
}

void *execute_dispatcher(void *args) {
	dispatcher *d = args;
	int done = 0;

	signal(SIGALRM, ALARM_handler_counter);
	alarm(d->M);

	while(done < d->sources) {
		done = 0;
		for(int i = 0; i < d->sources; i++) {
			// skip over full worker queues and constantly checks for work to be done
			if(!is_full(d->workers[i]->queue)) {
				HashPacket_t *p;
				p = (HashPacket_t *) getRandomPacket(&d->pks);
				enq(d->workers[i]->queue, p);
			}
			if(d->workers[i]->is_done) {
				done++;
			}
		}
	}

	for(int i = 0; i < d->sources; i++) {
		d->packets += d->workers[i]->p_remaining;
		d->fingerprint += d->workers[i]->fingerprint;
	}

	pthread_exit(NULL);
	return(NULL);
}
