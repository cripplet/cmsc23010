#include <math.h>
#include <stdio.h>

#include "counter.h"
#include "lock.h"
#include "tune.h"

#include "expr.h"

void counter_1() {
	int in_time;
	float out_count;
	int lock_mode;
	for(int t = 0; t <= 10; t++) {
		out_count = 0;
		in_time = pow(2, t);
		for(int iter = 0; iter < ITER; iter++) {
			out_count += time_counter_serial(in_time) / ITER;
		}
		fprintf(stderr, "%i\t%s\t%s\t%i\t%f\n", IDLE, "COUNTER_1", "serial", in_time, out_count);
	}
	for(int t = 0; t <= 10; t++) {
		for(int m = 0; m < 5; m++) {
			out_count = 0;
			in_time = pow(2, t);
			lock_mode = pow(2, m);
			for(int iter = 0; iter < ITER; iter++) {
				out_count += time_counter_parallel(in_time, 1, lock_mode);
			}
			fprintf(stderr, "%i\t%s\t%s\t%i\t%f\t%i\n", IDLE, "COUNTER_1", "parallel", in_time, out_count, lock_mode);
		}
	}
}

void counter_2() {
	int in_count;
	float out_time;
	int lock_mode;
	for(int b = 10; b <= 20; b++) {
		out_time = 0;
		in_count = pow(2, b);
		for(int iter = 0; iter < ITER; iter++) {
			out_time += work_counter_serial(in_count);
		}
		fprintf(stderr, "%i\t%s\t%s\t%f\t%i\n", IDLE, "COUNTER_2", "serial", out_time, in_count);
	}
	for(int b = 10; b <= 20; b++) {
		for(int m = 0; m < 5; m++) {
			out_time = 0;
			in_count = pow(2, b);
			lock_mode = pow(2, m);
			for(int iter = 0; iter < ITER; iter++) {
				out_time += work_counter_parallel(in_count, 1, lock_mode);
			}
			fprintf(stderr, "%i\t%s\t%s\t%f\t%i\t%i\n", IDLE, "COUNTER_1", "parallel", out_time, in_count, lock_mode);
		}
	}
}

void counter_3() {
}

void counter_4() {
}

void counter_5() {
}

void packet_1() {
}

void packet_2() {
}

void packet_3() {
}

void packet_4() {
}

void tune() {
	int in_time = 10000;
	float out_count;
	for(int min_d = 0; min_d < 10; min_d++) {
		for(int max_d = min_d; max_d < 10; max_d++) {
			min_delay = pow(2, min_d);
			max_delay = pow(2, max_d);
			out_count = 0;
			for(int iter = 0; iter < (ITER * ITER * ITER); iter++) {
				out_count += time_counter_parallel(in_time, 8, BACK) / (ITER * ITER * ITER);
			}
			fprintf(stderr, "%s\t%f\t%i\t%i\n", "TUNE", out_count, min_delay, max_delay);
		}
	}
}
