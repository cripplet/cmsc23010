#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#include "test.h"
#include "type.h"
#include "hash.h"

int main(int argc, char **argv) {
	int mode = TEST;
	if(argc == 2) {
		mode = atoi(argv[1]);
	} else {
		return(0);
	}

	switch(mode) {
		case TEST:
			fprintf(stderr, "running tests\n");
			int success = (
				test_hash(LOCKING)
			);
			fprintf(stderr, "test results: %s\n", success ? "PASS" : "FAIL");
			return(!success);
	}
	return(0);
}
