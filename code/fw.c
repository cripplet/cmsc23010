#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "fw.h"
#include "config.h"

struct blob {
	int * **arr_addr;
	int dim;
	pthread_barrier_t fence;
	int thread_count;
};

struct thread_data {
	int tid;
	int cur_row;
	int end_row;
	struct blob *shared_data;
};

struct thread_data *initialize_thread_data(int, struct blob *);
struct blob *initialize_blob(int * **, int, int);

void *thread_execute(void *);

void *thread_execute(void *args) {
	struct thread_data *td = (struct thread_data *) args;

	for(int k = 0; k < td->shared_data->dim; k++) {
		for(int i = td->cur_row; i < td->end_row; i++) {
			for(int j = 0; j < td->shared_data->dim; j++) {
				execute(i, j, k, *td->shared_data->arr_addr);
			}
		}
		pthread_barrier_wait(&td->shared_data->fence);
	}
	pthread_exit(NULL);
	return(NULL);
}

void execute_serial(int dim, int **arr) {
	for(int k = 0; k < dim; k++) {
		for(int i = 0; i < dim; i++) {
			for(int j = 0; j < dim; j++) {
				execute(i, j, k, arr);
			}
		}
	}
}

void execute_parallel(int dim, int **arr, int thread_count) {
	pthread_t *handler_list = calloc(dim, sizeof(pthread_t));

	struct blob *b = initialize_blob(&arr, dim, thread_count);

	for(int i = 0; i < thread_count; i++) {
		pthread_create(&handler_list[i], NULL, thread_execute, initialize_thread_data(i, b));
	}

	// wait for all threads to finish before exiting
	for(int i = 0; i < dim; i++) {
		pthread_join(handler_list[i], NULL);
	}
}

/**
 * For sake of speed, does NOT check if i, j, k > dim.
 *
 * It is up to the distributors to ensure this is the case.
 */
void execute(int i, int j, int k, int **arr) {
	if(arr[i][j] <= MAX_WEIGHT) {
		if(arr[i][j] > arr[k][i] + arr[k][j]) {
			arr[i][j] = arr[k][i] + arr[k][j];
		}
	}
}

struct thread_data *initialize_thread_data(int tid, struct blob *b) {
	struct thread_data *td = calloc(1, sizeof(struct thread_data));

	td->tid = tid;
	td->shared_data = b;
	td->cur_row = (b->dim / b->thread_count) * tid;
	td->end_row = (b->dim / b->thread_count) * (tid + 1);

	return(td);
}

struct blob *initialize_blob(int * **arr_addr, int dim, int thread_count) {
	struct blob *b = calloc(1, sizeof(struct blob));
	pthread_barrier_init(&b->fence, NULL, thread_count);
	b->arr_addr = arr_addr;
	b->dim = dim;
	b->thread_count = thread_count;

	return(b);
}
