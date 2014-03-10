#include "lock.h"
#include "type.h"

#include "hash.h"

/* Blob typedefs functions */
typedef struct locking_blob_t {
	int len;
	read_write_lock **locks;
} locking_blob;

typedef struct lockfree_blob_t {
} lockfree_blob;

typedef struct linear_blob_t {
} linear_blob;

typedef struct awesome_blob_t {
} awesome_blob;

/* Blob housekeeping functions */
locking_blob *ht_locking_blob_init(int max_s) {
	locking_blob *b = malloc(sizeof(locking_blob));
	b->len = max_s;
	b->locks = malloc(max_s * sizeof(read_write_lock *));
	for(int i = 0; i < max_s; i++) {
		b->locks[i] = read_write_lock_init();
	}
	return(b);
}

void ht_locking_blob_free(locking_blob *b) {
	for(int i = 0; i < b->len; i++) {
		read_write_lock_free(b->locks[i]);
	}
	free(b->locks);
	free(b);
}

/* Auxiliary function headers */
int ht_find(hash_table *t, packet_source *elem, int remove);

/* Memory allocation */
hash_table *ht_init(int type, int heur, int max_s) {
	hash_table *t = malloc(sizeof(hash_table));
	t->type = type;
	t->heur = heur;
	t->size = 0;
	t->max_s = max_s;
	switch(type) {
		case LOCKING:
			t->b = ht_locking_blob_init(max_s);
			break;
		default:
			t->b = NULL;
	}
	return(t);
}

void ht_free(hash_table *t) {
	switch(t->type) {
		case LOCKING:
			ht_locking_blob_free((locking_blob *) t->b);
			break;
		default:
			break;
	}
	free(t);
}

/* Table probing */
int ht_is_full(hash_table *t) {
	return(t->max_s >= t->size);
}

/* Sets the current size of the hash table */
int ht_set_size(hash_table *t) {
	int success = 0;
	int size = t->size;
	switch(t->type) {
		default:
			break;
	}
	if(success) {
		t->size = size;
	}
	return(success);
}

int ht_acquire(hash_table *t) {
	switch(t->type) {
		default:
			break;
	}
	return(1);
}

int ht_release(hash_table *t) {
	switch(t->type) {
		default:
			break;
	}
	return(1);
}

int ht_resize(hash_table *t) {
	int success = 0;
	switch(t->type) {
		default:
			break;
	}
	if(success) {
		t->max_s <<= 2;
	}
	return(success);
}

int ht_add(hash_table *t, packet_source *elem) {
	int success = 0;
	switch(t->type) {
		default:
			break;
	}
	// resize table if necessary
	if(ht_is_full(t)) {
		success &= ht_resize(t);
	}
	success &= ht_set_size(t);
	return(success);
}

int ht_remove(hash_table *t, packet_source *elem) {
	return(ht_find(t, elem, 1));
}

int ht_contains(hash_table *t, packet_source *elem) {
	return(ht_find(t, elem, 0));
}

int ht_find(hash_table *t, packet_source *elem, int remove) {
	int success = 0;
	switch(t->type) {
		default:
			break;
	}
	ht_set_size(t);
	return(success);
}
