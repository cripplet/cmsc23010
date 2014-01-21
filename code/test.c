#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "fw.h"
#include "io.h"
#include "test.h"
#include "utils.h"

struct test_data {
	char *name;

	// thread function pointer
	//	cf. http://bit.ly/1mo6mV1
	void * (*func) (void *);

	// expected exit status
	int expected_status;
};

/**
 * test structure house-keeping functions
 */
struct test_data *create_test_data(char*, void * (*) (void *), int);
int run_test(struct test_data *);

/**
 * test declarations
 */
void *failed_open(void *);
void *failed_format(void *);
void *successful_open(void *);
void *execute_increment(void *);
void *execute_stay(void *);
void *trivial_exec_serial(void *);
void *trivial_exec_parallel(void *);
void *compare_serial_parallel_exec(void *);

struct test_data *create_test_data(char* name, void * (*func) (void *), int expected_status) {
	struct test_data *data_struct = calloc(1, sizeof(struct test_data));
	data_struct->name = name;
	data_struct->func = func;
	data_struct->expected_status = expected_status;
	return(data_struct);
}

int run_test(struct test_data *test) {
	pthread_t tid;
	pthread_create(&tid, NULL, test->func, NULL);
	pthread_join(tid, NULL);

	int success = (*result == test->expected_status);

	if(success) {
		notice(test->name, "PASSED");
	} else {
		notice(test->name, "FAILED");
	}

	*result = SUCCESS;

	return(success);
}

int test_all() {
	return(
		run_test(create_test_data("failed_open", failed_open, ERR_OPEN)) &
		run_test(create_test_data("failed_format", failed_format, ERR_FORMAT)) &
		run_test(create_test_data("successful_open", successful_open, SUCCESS)) &
		run_test(create_test_data("execute_increment", execute_increment, SUCCESS)) &
		run_test(create_test_data("execute_stay", execute_stay, SUCCESS)) &
		run_test(create_test_data("trivial_exec_serial", trivial_exec_serial, SUCCESS)) &
		run_test(create_test_data("trivial_exec_parallel", trivial_exec_parallel, SUCCESS)) &
		run_test(create_test_data("compare_serial_parallel_exec", compare_serial_parallel_exec, SUCCESS))
	);
}

/**
 * File does not exist.
 */
void *failed_open(void *args) {
	read_array("invalid_file_name", NULL);
	return(NULL);
}

/**
 * File is in the wrong format.
 */
void *failed_format(void *args) {
	read_array("test_data/failed_format.txt", NULL);
	return(NULL);
}

/**
 * File can be read.
 */
void *successful_open(void *args) {
	int dim;
	int **arr = read_array("test_data/successful_open.txt", &dim);

	int comp_arr[4][4] = {
		{0, 1, 2, 3},
		{1, 0, 4, 5},
		{2, 4, 0, 6},
		{3, 5, 6, 0}
	};

	for(int i = 0; i < dim; i++) {
		for(int j = 0; j < dim; j++) {
			if(arr[i][j] != comp_arr[i][j]) {
				fail(ERROR, "successful_open", "Array did not increment correctly.");
			}
		}
	}
	return(NULL);
}

/**
 * Execute an incrementing move on a given array and test to ensure that the increment occurred.
 */
void *execute_increment(void *args) {
	int mat[3][3] = {
		{0, 1, 2},
		{1, 0, 5},
		{2, 5, 0}
	};
	int **arr = initialize_matrix(3, mat);

	int i = 1;
	int j = 2;
	int k = 0;

	execute(i, j, k, arr);
	if(arr[i][j] != 3) {
		fail(ERROR, "execute_increment", "Array did not increment correctly.");
	}
	return(NULL);
}

/**
 * Given an array, give appropriate i, j, k such that values are NOT incremented, and check to ensure this is the case.
 */
void *execute_stay(void *args) {
	int mat[3][3] = {
		{0, 1, 2},
		{1, 0, 5},
		{2, 5, 0}
	};
	int **arr = initialize_matrix(3, mat);

	int i = 0;
	int j = 2;
	int k = 1;

	execute(i, j, k, arr);
	if(arr[i][j] != 2) {
		fail(ERROR, "execute_increment", "Array incremented when conditions preclude it from being so.");
	}
	return(NULL);
}

void *trivial_exec_serial(void *args) {
	int mat[4][4] = {
		{0, 1, 1, 1},
		{1, 0, 1, 1},
		{1, 1, 0, 1},
		{1, 1, 1, 0}
	};
	int **arr = initialize_matrix(4, mat);

	execute_serial(4, arr);

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			if(mat[i][j] != arr[i][j]) {
				fail(ERROR, "trivial_exec_serial", "Serial execution did not match expected outcome.");
			}
		}
	}
	return(NULL);
}

void *trivial_exec_parallel(void *args) {
	int mat[4][4] = {
		{0, 1, 1, 1},
		{1, 0, 1, 1},
		{1, 1, 0, 1},
		{1, 1, 1, 0}
	};
	int **arr = initialize_matrix(4, mat);

	execute_parallel(4, arr, 4);

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			if(mat[i][j] != arr[i][j]) {
				fail(ERROR, "trivial_exec_parallel", "Parallel execution did not match expected outcome.");
			}
		}
	}
	return(NULL);
}

void *compare_serial_parallel_exec(void *args) {
	int mat[4][4] = {
		{0, 1, 4, 6},
		{1, 0, 4, 8},
		{4, 4, 0, 1},
		{6, 8, 1, 0}
	};
	int **arr_ser = initialize_matrix(4, mat);
	int **arr_par = initialize_matrix(4, mat);

	execute_serial(4, arr_ser);
	execute_parallel(4, arr_par, 4);

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			if(arr_ser[i][j] != arr_par[i][j]) {
				fail(ERROR, "compare_serial_parallel_exec", "Parallel results did not match serial execution result.");
			}
		}
	}

	return(NULL);
}
