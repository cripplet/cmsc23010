#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#include "tune.h"

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
			int result = (1
			);
			fprintf(stderr, "test results: %s\n", !result ? "PASS" : "FAIL");
			return(result);
	}
	return(0);
}
