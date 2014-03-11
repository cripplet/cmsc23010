#ifndef COUNTER_H
#define COUNTER_H

// M is in units of microseconds,
int time_counter_serial(int M);
int time_counter_parallel(int M, int n, int L);

// the float returned by work_counter_X is in microseconds
float work_counter_serial(int B);
float work_counter_parallel(int B, int n, int L);

#endif
