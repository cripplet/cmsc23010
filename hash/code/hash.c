#include <pthread.h>

#include "utils/packetsource.h"
#include "type.h"

#include "hash.h"

// TODO -- CANNOT ADD ITEMS WITH DUPLICATE KEYS IN HASH TABLE

/* Blob typedefs functions */
typedef struct locking_blob_t {
	int len;
	pthread_rwlock_t *locks;
} locking_blob;

typedef struct lockfree_blob_t {
} lockfree_blob;

typedef struct linear_blob_t {
} linear_blob;

typedef struct awesome_blob_t {
} awesome_blob;

/* Blob housekeeping functions */
locking_blob *ht_locking_blob_init(int log_threads) {
	locking_blob *b = malloc(sizeof(locking_blob));
	b->len = log_threads;
	b->locks = malloc(log_threads * sizeof(pthread_rwlock_t));
	for(int i = 0; i < log_threads; i++) {
		pthread_rwlock_init(&b->locks[i], NULL);
	}
	return(b);
}

void ht_locking_blob_free(locking_blob *b) {
	free(b->locks);
	free(b);
}

/* Auxiliary function headers */
void serial_list_free(serial_list *l);

/* Memory allocation */
hash_table *ht_init(int type, int heur, int log_threads) {
	hash_table *t = malloc(sizeof(hash_table));
	t->type = type;
	t->heur = heur;

	/* Hash table allocation */
	t->len = log_threads;
	t->buckets = malloc(log_threads * sizeof(serial_list *));
	for(int i = 0; i < log_threads; i++) {
		t->buckets[i] = createSerialList();
	}

	/* Set size constraints */
	t->size = 0;
	switch(heur) {
		case TABLE:
			t->max_s = log_threads << 1;
			break;
		default:
			t->max_s = 0;
			break;
	}

	switch(type) {
		case LOCKING:
			t->b = ht_locking_blob_init(log_threads);
			break;
		default:
			t->b = NULL;
			break;
	}

	t->mask = log_threads - 1;

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

	/* Hash table freeing */
	for(int i = 0; i < t->len; i++) {
		serial_list_free((serial_list *) t->buckets[i]);
	}
	free(t->buckets);

	free(t);
}

/* Table probing */
int ht_is_full(hash_table *t) {
	int size;
	switch(t->heur) {
		case TABLE:
			size = t->size;
			break;
		default:
			size = 0;
	}

	return(t->max_s < size);
}

int ht_resize(hash_table *t) {
	int success = 1;
	locking_blob *locking_b;

	// acquire all necessary locks
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			for(int i = 0; i < locking_b->len; i++) {
				pthread_rwlock_wrlock(&locking_b->locks[i]);
			}
		default:
			break;
	}

	/* Hash table allocation */
	int t_len = t->len << 1;
	int t_mask = (t->mask << 1) + 1;
	volatile serial_list **t_buckets = malloc(t_len * sizeof(serial_list *));
	for(int i = 0; i < t_len; i++) {
		t_buckets[i] = createSerialList();
	}

	/* Copy data */
	for(int i = 0; i < t->len; i++) {
		item *curr = t->buckets[i]->head;
		while(curr != NULL) {
			item *next = curr->next;
			curr->next = t_buckets[curr->key & t_mask]->head;
			t_buckets[curr->key & t_mask]->head = curr;
			curr = next;
		}
	}

	/* Free old hash table */
	for(int i = 0; i < t->len; i++) {
		free((serial_list *) t->buckets[i]);
	}

	free(t->buckets);

	t->buckets = t_buckets;
	t->len = t_len;
	t->mask = t_mask;

	if(success) {
		switch(t->heur) {
			case TABLE:
				t->max_s <<= 1;
				break;
			default:
				break;
		}
	}

	// release the locks
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			for(int i = 0; i < locking_b->len; i++) {
				pthread_rwlock_unlock(&locking_b->locks[i]);
			}
		default:
			break;
	}

	return(success);
}

/**
 * Return false if duplicate key found
 *
 * Caller is responsible for duplicate resources
 */
int ht_add(hash_table *t, int key, packet *elem) {
	int success = 0;
	int index = key & t->mask;
	locking_blob *locking_b;

	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_wrlock(&locking_b->locks[index % locking_b->len]);
			break;
		default:
			break;
	}
	success = !contains_list((serial_list *) t->buckets[index], key);
	if(success) {
		add_list((serial_list *) t->buckets[index], key, elem);
		switch(t->heur) {
			case TABLE:
				t->size += 1;
				break;
			default:
				break;
		}
	}
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
			break;
		default:
			break;
	}

	// resize table if necessary
	if(ht_is_full(t)) {
		success &= ht_resize(t);
	}
	return(success);
}

int ht_remove(hash_table *t, int key) {
	int success = 0;
	int index = key & t->mask;
	locking_blob *locking_b;

	if((index & t->mask) >= t->len) {
		return(success);
	}

	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_wrlock(&locking_b->locks[index % locking_b->len]);
			break;
		default:
			break;
	}
	success |= remove_list((serial_list *) t->buckets[index], key);
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
			break;
		default:
			break;
	}
	return(success);
}

int ht_contains(hash_table *t, int key) {
	int success = 0;
	int index = key & t->mask;
	locking_blob *locking_b;

	if((index & t->mask) >= t->len) {
		return(success);
	}

	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_rdlock(&locking_b->locks[index % locking_b->len]);
			break;
		default:
			break;
	}
	success |= contains_list((serial_list *) t->buckets[index], key);
	switch(t->type) {
		locking_blob *locking_b;
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
			break;
		default:
			break;
	}
	return(success);
}

void serial_list_free(serial_list *l) {
	item *i = l->head;
	item *temp;
	while(i != NULL) {
		temp = i->next;
		free((packet *) i->value);
		free(i);
		i = temp;
	}
	free(l);
}
