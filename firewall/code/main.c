#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serial.h"
#include "parallel.h"
#include "serial_queue.h"
#include "test.h"

#define DEFAULT_NUMBER_OF_ARGS 7

#define SERIAL 1
#define PARALLEL 2
#define SERIAL_QUEUE 3

float serial_firewall(const int, const int, const long, const int, const short);

int main(int argc, char * argv[]) {
       	int numPackets, numSources, uniformFlag, mode;
	long mean;
	short experimentNumber;

	float result = 0.;

	/* run tests */
	if(argc == 2) {
		return(!test_all());
	/* else run the firewall */
	} else if(argc == DEFAULT_NUMBER_OF_ARGS) {
        	numPackets = atoi(argv[1]);
		numSources = atoi(argv[2]);
		mean = atol(argv[3]);
		uniformFlag = atoi(argv[4]);
		experimentNumber = (short) atoi(argv[5]);
		mode = atoi(argv[6]);
	} else {
		fprintf(stderr, "usage: %s (TEST | <packets> <sources> <mean> <workload> <id> <mode>)\n\tmode: 0 ALL 1 SERIAL 2 PARALLEL 3 PARALLEL_QUEUE\n", argv[0]);
		return(0);
	}
	switch(mode) {
		case SERIAL:
			result = serial_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
			break;
		case PARALLEL:
			result = parallel_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
			break;
		case SERIAL_QUEUE:
			result = serial_queue_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
			break;
	}
	fprintf(stdout, "%s %i %i %li %i %i %i : %f\n", argv[0], numPackets, numSources, mean, uniformFlag, experimentNumber, mode, result);
	return(0);
}
