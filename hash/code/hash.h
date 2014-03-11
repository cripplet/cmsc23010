#ifndef HASH_T
#define HASH_T

#include "utils/seriallist.h"
#include "utils/packetsource.h"

typedef PacketSource_t packet_source;
typedef Packet_t packet;
typedef SerialList_t serial_list;
typedef Item_t item;

typedef struct hash_table_t {
	/* Locking structures */
	int type;			// designates the table TYPE (LOCKING, LOCKFREEC, LINEAR, AWESOME)
	void *b;			// the specific blob of the hash table with type = TYPE

	/* Hash table sizing */
	int heur;			// designates the heuristic type that the hash table will employ to determine if it is full
	volatile int size;		// current size of the hash table
	volatile int max_s;		// maximum size metric of the hash table
	volatile int mask;

	/* Hash table struct */
	volatile int len;		// the number of buckets
	volatile serial_list **buckets;	// the actual hash table


	pthread_rwlock_t lock;
} hash_table;

/* Memory allocation */
hash_table *ht_init(int type, int heur, int log_threads);
void ht_free(hash_table *t);

/* Table probing */
int ht_is_full(hash_table *t);
int ht_set_size(hash_table *t);
int ht_resize(hash_table *t);
int ht_add(hash_table *t, int key, packet *elem);
int ht_remove(hash_table *t, int key);
int ht_contains(hash_table *t, int key);

#endif
