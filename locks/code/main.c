#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#include "expr.h"
#include "lock.h"
#include "test.h"
#include "tune.h"

int main(int argc, char **argv) {
	min_delay = 16;
	max_delay = 128;

	int mode = TEST;
	if(argc == 2) {
		mode = atoi(argv[1]);
	} else {
		fprintf(stderr, "usage: %s <MODE>\n\tTEST:	run test suite\n", argv[0]);
	}

	switch(mode) {
		case TEST:
			fprintf(stderr, "running tests\n");
			int result = (
				test_lock(TTAS) &
				test_lock(BACK) &
				test_lock(MUTX) &
				test_lock(ALCK) &
				test_lock(CLHQ));
			fprintf(stderr, "test_lock: %s\n", !result ? "PASS" : "FAIL");
			return(result);

		case IDLE:
			counter_1();
			counter_2();
			break;

		case SCAL:
			counter_3();
			counter_4();
			break;

		case FAIR:
			counter_5();
			break;

		case POHD:
			packet_1();
			break;

		case PSCL:
			packet_2();
			packet_3();
			packet_4();
			break;
		case TUNE:
			tune();
	}
	return(0);
}
