#ifndef HASH_T
#define HASH_T

#include "utils/packetsource.h"

typedef PacketSource_t packet_source;

typedef struct hash_table_t {
	int type;		// designates the table TYPE (LOCKING, LOCKFREE, LINEAR, AWESOME)
	void *b;		// the specific blob of the hash table with type = TYPE
	int heur;		// designates the heuristic type that the hash table will employ to determine if it is full
	int size;		// current size of the hash table -- referenced in is_full()
	int max_s;		// maximum size of the hash table
} hash_table;

/* Memory allocation */
hash_table *ht_init(int type, int heur, int max_s);
void ht_free(hash_table *t);

/* Table probing */
int ht_is_full(hash_table *t);
int ht_set_size(hash_table *t);
int ht_resize(hash_table *t);
int ht_add(hash_table *t, packet_source *elem);
int ht_remove(hash_table *t, packet_source *elem);
int ht_contains(hash_table *t, packet_source *elem);

#endif
