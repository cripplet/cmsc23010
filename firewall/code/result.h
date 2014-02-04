#ifndef RESULT_H
#define RESULT_H

typedef struct result_h {
	float time;
	float folded_time;
	long fingerprint;
} result;

result *init_result();

#endif
