#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/generators.h"
#include "utils/stopwatch.h"
#include "utils/fingerprint.h"
#include "utils/packetsource.h"

#include "serial.h"

result *serial_firewall(int numPackets, int numSources, long mean, int uniformFlag, short experimentNumber) {
	PacketSource_t * packetSource = createPacketSource(mean, numSources, experimentNumber);
	StopWatch_t watch;
	long fingerprint = 0;

	if(uniformFlag) {
		startTimer(&watch);
		for( int i = 0; i < numSources; i++ ) {
			for( int j = 0; j < numPackets; j++ ) {
				volatile Packet_t * tmp = getUniformPacket(packetSource,i);
				fingerprint += getFingerprint(tmp->iterations, tmp->seed);
			}
		}
		stopTimer(&watch);
	} else {
		startTimer(&watch);
		for( int i = 0; i < numSources; i++ ) {
			for( int j = 0; j < numPackets; j++ ) {
				volatile Packet_t * tmp = getExponentialPacket(packetSource,i);
				fingerprint += getFingerprint(tmp->iterations, tmp->seed);
			}
		}
		stopTimer(&watch);
	}

	result *r = init_result();
	r->time = getElapsedTime(&watch);
	return(r);
}
