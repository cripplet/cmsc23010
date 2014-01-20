#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "utils.h"

/**
 * Exits with given status and message.
 */
void fail(int status, char *caller, char *message) {
	fprintf(stderr, "%s: %s\n", caller, message);
	if(!TEST) {
		exit(status);
	} else {
		*result = status;
		pthread_exit((void *) result);
	}
}

/**
 * Prints a message -- if TEST is turned off, raise an error instead.
 */
void debug(char *caller, char *message) {
	fprintf(stderr, "debug -- %s: %s\n", caller, message);
	if(!TEST) {
		fail(ERROR, caller, "Debug statement executed while in production.");
	}
}

/**
 * Prints a status message.
 */
void notice(char *caller, char *message) {
	fprintf(stdout, "%s: %s\n", caller, message);
}

/**
 * Create a double pointer from the given matrix.
 */
int **initialize_matrix(int dim, int arr[dim][dim]) {
	int **matrix = calloc(dim, sizeof(int *));
	for(int i = 0; i < dim; i++) {
		matrix[i] = calloc(dim, sizeof(int *));
	}
	for(int j = 0; j < dim; j++) {
		for(int k = 0; k < dim; k++) {
			matrix[j][k] = arr[j][k];
		}
	}
	return(matrix);
}

/**
 * Prints out the usage for the executable.
 */
void usage(char *exec) {
	fprintf(stderr, "usage: %s -n <int> [ -c <int> ] [ -i <int> ]\n\t-n\tsize of array; strictly positive\n\t-c\tnumber of threads to execute on; defaults to serial execution; max threads equal to n\n\t-i\tloop ID for multiple executions; default to 0\n", exec);
}

/**
 * Outputs all relevant information about the execution to a log file.
 */
void fw_log(int id, char *start_time, char *in, char *out, int dim, int thread_count, double time) {
	FILE *fp = fopen("log.txt", "a");
	if(fp != NULL) {
		strtok(start_time, "\n");
		fprintf(fp, "%i\t%s\t%s\t%s\t%i\t%i\t%f\n", id, start_time, in, out, dim, thread_count, time);
		fclose(fp);
	}
}
