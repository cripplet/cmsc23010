/**
 * This is NOT used in project 4 -- cf. utils/hashpackettest.c for the serial implementation
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/hashgenerator.h"
#include "utils/generators.h"
#include "utils/fingerprint.h"
#include "utils/packetsource.h"
#include "utils/hashtable.h"

#include "counter.h"
#include "tune.h"

#include "serial.h"

result *serial_firewall(int log_threads, int numSources, long mean, short experimentNumber, int M) {
	PacketSource_t * packetSource = createPacketSource(mean, numSources, experimentNumber);
	long fingerprint = 0;
	long packets = 0;
	SerialHashTable_t *t = createSerialHashTable(log_threads, MAX_BUCKET_SIZE);

	HashPacket_t *tmp;

	signal(SIGALRM, ALARM_handler_counter);
	alarm(M);

	while(b->flags) {
		for(int i = 0; i < numSources; i++) {
			if(UNIFORM) {
				tmp = (HashPacket_t *) getUniformPacket(packetSource, i);
			} else {
				tmp = (HashPacket_t *) getExponentialPacket(packetSource, i);
			}
			long f = getFingerprint(tmp->body->iterations, tmp->body->seed);

			// TODO -- replace with the appropriate function
			add_ht(t, f, NULL);

			fingerprint += f;
			packets += 1;
		}
	}

	result *r = init_result();

	r->packets = packets;
	r->fingerprint = fingerprint;

	return(r);
}
