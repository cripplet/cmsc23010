#include "utils/stopwatch.h"
#include "utils/packetsource.h"
#include "queue.h"
#include "worker.h"

#include "serial_queue.h"

result *serial_queue_firewall(int numPackets, int numSources, long mean, int uniformFlag, short experimentNumber) {
	worker **workers = malloc(numSources * sizeof(worker *));
	for(int i = 0; i < numSources; i++) {
		workers[i] = init_worker(numPackets, Q_SIZE);
	}

	PacketSource_t *pks = createPacketSource(mean, numSources, experimentNumber);

	StopWatch_t watch;
	startTimer(&watch);
	for(int i = 0; i < numPackets; i++) {
		for(int j = 0; j < numSources; j++) {
			Packet_t *p;
			if(uniformFlag) {
				p = (Packet_t *) getUniformPacket(pks, j);
			} else {
				p = (Packet_t *) getExponentialPacket(pks, j);
			}
			enq(workers[j]->queue, p);
			workers[j]->fingerprint += process_packet(workers[j]);
		}
	}
	stopTimer(&watch);

	result *r = init_result();
	r->time = getElapsedTime(&watch);

	for(int i = 0; i < numSources; i++) {
		r->fingerprint += workers[i]->fingerprint;
	}

	return(r);
}
