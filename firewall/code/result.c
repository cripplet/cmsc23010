#include <stdlib.h>

#include "result.h"

result *init_result() {
	result *r = malloc(sizeof(result));
	r->time = 0;
	r->folded_time = 0;
	r->fingerprint = 0;
	return(r);
}
