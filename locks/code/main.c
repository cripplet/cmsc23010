#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#include "expr.h"
#include "lock.h"
#include "test.h"

int main(int argc, char **argv) {
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
				test_lock(ALCK));
			fprintf(stderr, "test_lock: %s\n", !result ? "PASS" : "FAIL");
			return(result);
		case IDLE:
		case SCAL:
		case FAIR:
		case POHD:
		case PSCL:
			break;
	}
	return(0);
}
