#include "hash.h"

/* Memory allocation */
hash_table *ht_init(int type, int heur, int max_s) {
	return(NULL);
}

void ht_free(hash_table *t) {
}

/* Table probing */
int ht_is_full(hash_table *t) {
	return(0);
}

int ht_set_size(hash_table *t) {
	return(0);
}

int ht_resize(hash_table *t) {
	return(0);
}

int ht_add(hash_table *t, packet_source *elem) {
	return(0);
}

int ht_remove(hash_table *t, packet_source *elem) {
	return(0);
}

int ht_contains(hash_table *t, packet_source *elem) {
	return(0);
}
