#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serial.h"
#include "parallel.h"
#include "serial_queue.h"
#include "test.h"
#include "result.h"

#define DEFAULT_NUMBER_OF_ARGS 7

#define SERIAL 1
#define PARALLEL 2
#define SERIAL_QUEUE 3

int main(int argc, char * argv[]) {
       	int numPackets, numSources, uniformFlag, mode;
	long mean;
	short experimentNumber;

	result *r = NULL;

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
			r = serial_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
			break;
		case PARALLEL:
			r = parallel_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
			break;
		case SERIAL_QUEUE:
			r = serial_queue_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
			break;
	}
	FILE *fp = fopen("log.txt", "a");
	if(mode == PARALLEL) {
		fprintf(fp, "%s\t%i\t%i\t%li\t%i\t%i\t%i\t%f\n", argv[0], numPackets, numSources, mean, uniformFlag, experimentNumber, mode, r->folded_time);
	} else {
		fprintf(fp, "%s\t%i\t%i\t%li\t%i\t%i\t%i\t%f\n", argv[0], numPackets, numSources, mean, uniformFlag, experimentNumber, mode, r->time);
	}
	fclose(fp);

	free(r);

	return(0);
}
