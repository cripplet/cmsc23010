#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "counter.h"
#include "lock.h"
#include "tune.h"
#include "parallel.h"
#include "serial.h"

#include "expr.h"

#define HUNDRED_MS 100000
#define MILLION 1000000
#define PACKETS 524288

void counter_1() {
	int in_time;
	float out_count;
	int lock_mode;
	for(int t = 0; t <= 10; t++) {
		out_count = 0;
		in_time = pow(2, t);
		for(short iter = 0; iter < ITER; iter++) {
			out_count += time_counter_serial(in_time) / ITER;
		}
		fprintf(stderr, "%i\t%s\t%s\t%i\t%f\n", IDLE, "COUNTER_1", "serial", in_time, out_count);
	}
	for(int t = 0; t <= 10; t++) {
		for(int m = 0; m < 5; m++) {
			out_count = 0;
			in_time = pow(2, t);
			lock_mode = pow(2, m);
			for(short iter = 0; iter < ITER; iter++) {
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
		for(short iter = 0; iter < ITER; iter++) {
			out_time += work_counter_serial(in_count);
		}
		fprintf(stderr, "%i\t%s\t%s\t%f\t%i\n", IDLE, "COUNTER_2", "serial", out_time, in_count);
	}
	for(int b = 10; b <= 20; b++) {
		for(int m = 0; m < 5; m++) {
			out_time = 0;
			in_count = pow(2, b);
			lock_mode = pow(2, m);
			for(short iter = 0; iter < ITER; iter++) {
				out_time += work_counter_parallel(in_count, 1, lock_mode);
			}
			fprintf(stderr, "%i\t%s\t%s\t%f\t%i\t%i\n", IDLE, "COUNTER_2", "parallel", out_time, in_count, lock_mode);
		}
	}
}

void counter_3() {
	float out_count;
	int n;
	int lock_mode;
	for(int m = 0; m < 5; m++) {
		for(int n_base = 0; n_base < 7; n_base++) {
			lock_mode = pow(2, m);
			n = pow(2, n_base);
			out_count = 0;
			for(short iter = 0; iter < ITER; iter++) {
				out_count += time_counter_parallel(HUNDRED_MS, n, lock_mode) / ITER;
			}
			fprintf(stderr, "%i\t%s\t%i\t%f\t%i\n", SCAL, "COUNTER_3", n, out_count, lock_mode);
		}
	}
}

void counter_4() {
	float out_time;
	int n;
	int lock_mode;
	for(int m = 0; m < 5; m++) {
		lock_mode = pow(2, m);
		out_time = 0;
		for(int n_base = 0; n_base < 7; n_base++) {
			n = pow(2, n_base);
			for(short iter = 0; iter < ITER; iter++) {
				out_time += work_counter_parallel(MILLION, n, lock_mode) / ITER;
			}
			fprintf(stderr, "%i\t%s\t%i\t%f\t%i\n", SCAL, "COUNTER_4", n, out_time, lock_mode);
		}
	}
}

void counter_5() {
	counter_5_result = malloc(sizeof(counter_result));
	counter_5_result->n = CORES;
	counter_5_result->contributions = malloc(CORES * sizeof(int));

	int lock_mode;
	float out_count;
	float std_dev;
	for(int m = 0; m < 5; m++) {
		lock_mode = pow(2, m);
		out_count = 0;
		std_dev = 0;
		for(int iter = 0; iter < ITER; iter++) {
			int tmp_count = time_counter_parallel(HUNDRED_MS, CORES, lock_mode);
			out_count += tmp_count / ITER;
			float mean = tmp_count / CORES;
			float variance = 0;
			for(int i = 0; i < counter_5_result->n; i++) {
				variance += pow(counter_5_result->contributions[i] - mean, 2);
			}
			std_dev += variance / ITER;
		}
		fprintf(stderr, "%i\t%s\t%f\t%f\t%i\n", FAIR, "COUNTER_5", out_count, std_dev, lock_mode);
	}
}

void packet_1() {
	int lock_mode;
	int strategy;
	long work;
	float out_time;
	for(int m = 0; m < 5; m++) {
		for(int w = 0; w < 6; w++) {
			for(int s = 0; s < 2; s++) {
				lock_mode = pow(2, m);
				strategy = pow(2, s);
				work = 25 * pow(2, w);
				out_time = 0;
				for(short iter = 0; iter < ITER; iter++) {
					out_time += (parallel_firewall(PACKETS, 1, work, 1, iter, lock_mode, strategy)->folded_time) / ITER;
				}
				fprintf(stderr, "%i\t%s\t%f\t%li\t%i\t%i\n", POHD, "PACKET_1", out_time, work, lock_mode, strategy);
			}
		}
	}
}

void loop(char *header, int packets, int n, long work, int uniform, int lock_mode, int strategy) {
	float out_time = 0;
	for(short iter = 0; iter < ITER; iter++) {
		out_time += (parallel_firewall(packets, n, work, uniform, iter, lock_mode, strategy)->folded_time) / ITER;
	}
	fprintf(stderr, "%i\t%s\t%s\t%i\t%f\t%li\t%i\t%i\t%i\n", PSCL, header, "parallel", packets, out_time, work, lock_mode, strategy, n);

	out_time = 0;
	for(short iter = 0; iter < ITER; iter++) {
		out_time += (serial_firewall(PACKETS, n, work, 1, iter)->time) / ITER;
	}
	fprintf(stderr, "%i\t%s\t%s\t%i\t%f\t%li\t%i\t%i\t%i\n", PSCL, header, "serial", packets, out_time, work, lock_mode, strategy, n);
}

void packet_2() {
	int lock_mode;
	int strategy;
	int work;
	for(int m = 0; m < 5; m++) {
		for(int w = 0; w < 4; w++) {
			for(int s = 0; s < 4; s++) {
				if(s != 1) {
					lock_mode = pow(2, m);
					strategy = pow(2, s);
					work = 1000 * pow(2, w);
					loop("PACKET_2", PACKETS, 1, work, 1, lock_mode, strategy);
					loop("PACKET_2", PACKETS, 2, work, 1, lock_mode, strategy);
					loop("PACKET_2", PACKETS, 3, work, 1, lock_mode, strategy);
					loop("PACKET_2", PACKETS, 7, work, 1, lock_mode, strategy);
					loop("PACKET_2", PACKETS, 15, work, 1, lock_mode, strategy);
				}
			}
		}
	}
}

void packet_3() {
	int lock_mode;
	int strategy;
	int work;
	for(int m = 0; m < 5; m++) {
		for(int w = 0; w < 4; w++) {
			for(int s = 0; s < 4; s++) {
				if(s != 1) {
					lock_mode = pow(2, m);
					strategy = pow(2, s);
					work = 1000 * pow(2, w);
					loop("PACKET_3", PACKETS, 1, work, 0, lock_mode, strategy);
					loop("PACKET_3", PACKETS, 2, work, 0, lock_mode, strategy);
					loop("PACKET_3", PACKETS, 3, work, 0, lock_mode, strategy);
					loop("PACKET_3", PACKETS, 7, work, 0, lock_mode, strategy);
					loop("PACKET_3", PACKETS, 15, work, 0, lock_mode, strategy);
				}
			}
		}
	}
}

void packet_4() {
	int lock_mode;
	int strategy;
	int work;
	int n;
	for(int m = 0; m < 5; m++) {
		for(int w = 0; w < 4; w++) {
			for(int s = 0; s < 4; s++) {
				for(int n_base = 0; n_base < 5; n_base++) {
					lock_mode = pow(2, m);
					strategy = pow(2, s);
					work = 1000 * pow(2, w);
					n = pow(2, n_base);
					loop("PACKET_4", PACKETS, n, work, 0, lock_mode, strategy);
				}
			}
		}
	}
}

void tune() {
	float out_count;
	for(int min_d = 0; min_d < 10; min_d++) {
		for(int max_d = min_d; max_d < 10; max_d++) {
			min_delay = pow(2, min_d);
			max_delay = pow(2, max_d);
			out_count = 0;
			for(short iter = 0; iter < (ITER * ITER * ITER); iter++) {
				out_count += time_counter_parallel(HUNDRED_MS / 10, 8, BACK) / (ITER * ITER * ITER);
			}
			fprintf(stderr, "%s\t%f\t%i\t%i\n", "TUNE", out_count, min_delay, max_delay);
		}
	}
}
