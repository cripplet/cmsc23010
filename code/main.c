#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "fw.h"
#include "io.h"
#include "main.h"
#include "utils.h"
#include "stopwatch.h"
#include "test.h"

// cf. http://bit.ly/1e4yNA4
char *get_time() {
	time_t now;
	struct tm *timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return(asctime(timeinfo));
}

int main(int argc, char **argv) {
	// unit testing
	if(TEST == 1) {
		result = calloc(1, sizeof(int));
		if(!test_all()) {
			fail(ERROR, "main", "One or more unit tests failed - exiting.");
		} else {
			notice("main", "All tests have passed, continue booting.");
		}
	}

	// keeps a record of the start of the TOTAL EXECUTION
	char *start_time = get_time();

	// thread_count represents number of threads to use, or SERIAL if thread execution is to be serial
	int thread_count = SERIAL;

	// size of the array
	int dim = 0;

	// additional identifier of the current running execution
	int id = 0;

	// exit if test_only after unit tests
	int test_only = 0;

	char input_fn[MAX_LINE], output_fn[MAX_LINE];

	int opt;
	while((opt = getopt(argc, argv, "n:c:i:t")) != -1) {
		switch(opt) {
			case 'n':
				dim = strtol(strdup(optarg), NULL, 10);
				break;
			case 'c':
				thread_count = strtol(strdup(optarg), NULL, 10);
				break;
			case 'i':
				id = strtol(strdup(optarg), NULL, 10);
				break;
			case 't':
				test_only = 1;
				break;
			default:
				usage(argv[0]);
				fail(ERR_ARGS, "main", "Unknown option provided.");
		}
	}

	if(test_only) {
		notice("main", "Running in test mode -- exiting.");
		return(0);
	}

	if(dim <= 0) {
		usage(argv[0]);
		fail(ERR_ARGS, "main", "Invalid array dimension.");
	}
	if((thread_count < 0) || (thread_count > dim)) {
		usage(argv[0]);
		fail(ERR_ARGS, "main", "Invalid thread count.");
	}

	sprintf(input_fn, "output_data/in_i%i_n%i.txt", id, dim);
	sprintf(output_fn, "output_data/out_i%i_n%i_c%i.txt", id, dim, thread_count);

	// get the array
	int **arr;
	arr = read_array(input_fn, &dim);

	notice("main", "Array loaded into memory.");
	notice("main", "Executing algorithm for the specified number of threads.");

	StopWatch_t *sw = initialize_stopwatch();

	/**
	 * Run the Floyd-Warshall algorithm with
	 * 1.) serial execution
	 * 2.) single thread execution
	 * 3.) multiple thread execution
	 *
	 * Analyze the cases in which 1 vs. 2 as function of N
	 * Analyze 2 vs. 3 as function of N
	 */

	switch(thread_count) {
		case SERIAL:
			startTimer(sw);
			execute_serial(dim, arr);
			stopTimer(sw);
			break;
		default:
			startTimer(sw);
			execute_parallel(dim, arr, thread_count);
			stopTimer(sw);
			break;
	}

	notice("main", "Finished execution -- writing to output file.");

	double time = getElapsedTime(sw);
	fw_log(id, start_time, input_fn, output_fn, dim, thread_count, time);

	// write results to output file
	write_array(output_fn, dim, arr);

	notice("main", "Exiting program.");
	return(SUCCESS);
}
