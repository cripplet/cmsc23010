#include <stdio.h>

#include <pthread.h>

#include "utils/packetsource.h"
#include "type.h"

#include "hash.h"

/* Blob typedefs functions */
typedef struct locking_blob_t {
	int len;
	pthread_rwlock_t *locks;
} locking_blob;

typedef struct lockfreec_blob_t {
	int len;
	pthread_mutex_t *locks;
} lockfreec_blob;

typedef struct linear_blob_t {
	int len;
	pthread_mutex_t *locks;
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

lockfreec_blob *ht_lockfreec_blob_init(int log_threads) {
	lockfreec_blob *b = malloc(sizeof(lockfreec_blob));
	b->len = log_threads;
	b->locks = malloc(log_threads * sizeof(pthread_mutex_t));
	for(int i = 0; i < log_threads; i++) {
		pthread_mutex_init(&b->locks[i], NULL);
	}
	return(b);
}

linear_blob *ht_linear_blob_init(int log_threads) {
	linear_blob *b = malloc(sizeof(linear_blob));
	b->len = log_threads;
	b->locks = malloc(log_threads * sizeof(pthread_mutex_t));
	for(int i = 0; i < log_threads; i++) {
		pthread_mutex_init(&b->locks[i], NULL);
	}
	return(b);
}

void ht_locking_blob_free(locking_blob *b) {
	free(b->locks);
	free(b);
}

void ht_lockfreec_blob_free(lockfreec_blob *b) {
	free(b->locks);
	free(b);
}

void ht_linear_blob_free(linear_blob *b) {
	free(b->locks);
	free(b);
}

linear_element *linear_element_init() {
	linear_element *elem = malloc(sizeof(linear_element));
	elem->key = 0;
	elem->offset = 0;
	elem->value = NULL;
	return(elem);
}

/* Auxiliary function headers */
void serial_list_free(serial_list *l);
void linear_element_free(linear_element *elem);

/* Memory allocation */
hash_table *ht_init(int type, int heur, int log_threads) {
	hash_table *t = malloc(sizeof(hash_table));
	t->type = type;
	t->heur = heur;

	/* Hash table allocation */
	t->len = log_threads;
	switch(type) {
		case LINEAR:
			t->elems = malloc(log_threads * sizeof(linear_element *));
			for(int i = 0; i < log_threads; i++) {
				t->elems[i] = linear_element_init();
			}
			break;
		case LOCKING:
		case LOCKFREEC:
		default:
			t->buckets = malloc(log_threads * sizeof(serial_list *));
			for(int i = 0; i < log_threads; i++) {
				t->buckets[i] = createSerialList();
			}
			break;
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
	if(type == LINEAR) {
		t->max_s = log_threads;
	}

	switch(type) {
		case LOCKING:
			t->b = ht_locking_blob_init(log_threads);
			break;
		case LOCKFREEC:
			t->b = ht_lockfreec_blob_init(log_threads);
			break;
		case LINEAR:
			t->b = ht_linear_blob_init(log_threads);
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
		case LOCKFREEC:
			ht_lockfreec_blob_free((lockfreec_blob *) t->b);
			break;
		case LINEAR:
			ht_linear_blob_free((linear_blob *) t->b);
			break;
		default:
			break;
	}

	switch(t->type) {
		case LINEAR:
			for(int i = 0; i < t->len; i++) {
				linear_element_free((linear_element *) t->elems[i]);
			}
			free(t->elems);
			break;
		default:
			/* Hash table freeing */
			for(int i = 0; i < t->len; i++) {
				serial_list_free((serial_list *) t->buckets[i]);
			}
			free(t->buckets);
			break;
	}

	free(t);
}

/* Table probing */
int ht_is_full(hash_table *t) {
	int size = 0;
	switch(t->heur) {
		case TABLE:
			size = t->size;
			break;
		case CONST:
			for(int i = 0; i < t->len; i++) {
				size = (size > t->buckets[i]->size) ? size : t->buckets[i]->size;
			}
			break;
		default:
			size = 0;
			break;
	}
	if(t->type == LINEAR) {
		size = t->size;
	}

	return(t->max_s <= size);
}

int ht_attempt_resize(hash_table *t) {
	int success = 1;
	locking_blob *locking_b;
	lockfreec_blob *lockfreec_b;
	linear_blob *linear_b;

	int old_length = t->len;

	// acquire all necessary locks
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			for(int i = 0; i < locking_b->len; i++) {
				pthread_rwlock_wrlock(&locking_b->locks[i]);
			}
			break;
		case LOCKFREEC:
			lockfreec_b = t->b;
			for(int i = 0; i < lockfreec_b->len; i++) {
				pthread_mutex_lock(&lockfreec_b->locks[i]);
			}
			break;
		case LINEAR:
			linear_b = t->b;
			for(int i = 0; i < linear_b->len; i++) {
				pthread_mutex_lock(&linear_b->locks[i]);
			}
		default:
			break;
	}

	if(t->len == old_length) {
		int t_len = t->len << 1;
		int t_mask = (t->mask << 1) + 1;
		if(t->type != LINEAR) {
			/* Hash table allocation */
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
		} else {
			volatile linear_element **t_elems = malloc(t_len * sizeof(linear_element *));
			for(int i = 0; i < t_len; i++) {
				t_elems[i] = linear_element_init();
			}
			for(int i = 0; i < t->len; i++) {
				if(t->elems[i]->value) {
					int index = t->elems[i]->key & t_mask;
					for(int offset = 0; offset < t_len;  offset++) {
						if(!t_elems[(index + offset) % t_len]->value) {
							t_elems[index % t_len]->offset = (t_elems[index % t_len]->offset > offset) ? t_elems[index % t_len]->offset : offset;
							t_elems[(index + offset) % t_len]->key = t->elems[i]->key;
							t_elems[(index + offset) % t_len]->value = t->elems[i]->value;
							break;
						}
					}
				}
			}
			for(int i = 0; i < t->len; i++) {
				linear_element_free((linear_element *) t->elems[i]);
			}
			free(t->elems);
			t->elems = t_elems;
		}

		t->len = t_len;
		t->mask = t_mask;

		if(success) {
			if(t->type != LINEAR) {
				switch(t->heur) {
					case TABLE:
					case CONST:
						t->max_s <<= 1;
						break;
					default:
						break;
				}
			} else {
				t->max_s <<= 1;
			}
		}
	}

	// release the locks
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			for(int i = 0; i < locking_b->len; i++) {
				pthread_rwlock_unlock(&locking_b->locks[i]);
			}
			break;
		case LOCKFREEC:
			lockfreec_b = t->b;
			for(int i = 0; i < lockfreec_b->len; i++) {
				pthread_mutex_unlock(&lockfreec_b->locks[i]);
			}
			break;
		case LINEAR:
			linear_b = t->b;
			for(int i = 0; i < linear_b->len; i++) {
				pthread_mutex_unlock(&linear_b->locks[i]);
			}
			break;
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

	while(ht_is_full(t)) {
		success &= ht_attempt_resize(t);
	}

	locking_blob *locking_b;
	lockfreec_blob *lockfreec_b;
	linear_blob *linear_b;

	int offset = 0;

	int old_len = t->len;
	int index = key & t->mask;

	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_wrlock(&locking_b->locks[index % locking_b->len]);
			break;
		case LOCKFREEC:
			lockfreec_b = t->b;
			pthread_mutex_lock(&lockfreec_b->locks[index % lockfreec_b->len]);
			break;
		case LINEAR:
			linear_b = t->b;
			// for(offset = t->elems[index]->offset; offset < t->len; offset++) {
			for(offset = 0; offset < t->len; offset++) {
				pthread_mutex_lock(&linear_b->locks[(index + offset) % linear_b->len]);
				// unlock if the potential spot is occupied
				if(t->elems[(index + offset) % t->len]->value) {
					pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
					// return if duplicate key found -- this is guaranteed to happen if a duplicate exists
					if(t->elems[(index + offset) % t->len]->key == key) {
						return(success);
					}
				} else {
					break;
				}
			}
		default:
			break;
	}
	if(old_len != t->len) {
		switch(t->type) {
			case LOCKING:
				locking_b = t->b;
				pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
				break;
			case LOCKFREEC:
				lockfreec_b = t->b;
				pthread_mutex_unlock(&lockfreec_b->locks[index % lockfreec_b->len]);
				break;
			case LINEAR:
				linear_b = t->b;
				pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
				break;
			default:
				break;
		}
		return(ht_add(t, key, elem));
	}
	if(t->type != LINEAR) {
		success |= !contains_list((serial_list *) t->buckets[index], key);
		if(success) {
			add_list((serial_list *) t->buckets[index], key, elem);

			switch(t->heur) {
				case TABLE:
					t->size += 1;
					break;
				case CONST:
					t->buckets[index]->size += 1;
					break;
				default:
					break;
			}
		}
	} else {
		t->elems[(index + offset) % t->len]->key = key;
		t->elems[(index + offset) % t->len]->value = elem;
		// note that the offset can ONLY grow -- as such, it is a MINIMUM starting point for all searches (and thus, does NOT need to be locked)
		t->elems[index % t->len]->offset = (t->elems[index % t->len]->offset > offset) ? t->elems[index % t->len]->offset : offset;
		t->size += 1;
		success |= 1;
	}
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
			break;
		case LOCKFREEC:
			lockfreec_b = t->b;
			pthread_mutex_unlock(&lockfreec_b->locks[index % lockfreec_b->len]);
			break;
		case LINEAR:
			linear_b = t->b;
			pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
			break;
		default:
			break;
	}

	return(success);
}

int ht_remove(hash_table *t, int key) {
	int success = 0;
	int index = key & t->mask;

	if((index & t->mask) >= t->len) {
		return(success);
	}

	locking_blob *locking_b;
	lockfreec_blob *lockfreec_b;
	linear_blob *linear_b;

	int offset = 0;

	int old_len = t->len;

	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_wrlock(&locking_b->locks[index % locking_b->len]);
			break;
		case LOCKFREEC:
			lockfreec_b = t->b;
			pthread_mutex_lock(&lockfreec_b->locks[index % lockfreec_b->len]);
			break;
		case LINEAR:
			linear_b = t->b;
			// for(offset = t->elems[index]->offset; offset < t->len; offset++) {
			for(offset = 0; offset < t->len; offset++) {
				pthread_mutex_lock(&linear_b->locks[(index + offset) % linear_b->len]);
				// unlock if not a match
				if(!t->elems[(index + offset) % t->len]->value || (t->elems[(index + offset) % t->len]->key != key)) {
					pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
				} else {
					success |= 1;
					break;
				}
			}
		default:
			break;
	}
	if(old_len != t->len) {
		switch(t->type) {
			case LOCKING:
				locking_b = t->b;
				pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
				break;
			case LOCKFREEC:
				lockfreec_b = t->b;
				pthread_mutex_unlock(&lockfreec_b->locks[index % lockfreec_b->len]);
				break;
			case LINEAR:
				linear_b = t->b;
				pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
				break;
			default:
				break;
		}
		return(ht_remove(t, key));
	}
	if(t->type != LINEAR) {
		success |= remove_list((serial_list *) t->buckets[index], key);
		if(success) {
			switch(t->heur) {
				case TABLE:
					t->size -= 1;
					break;
				case CONST:
					t->buckets[index]->size -= 1;
					break;
				default:
					break;
			}
		}
	} else {
		if(success) {
			free((packet *) t->elems[(index + offset) % t->len]->value);
			t->elems[(index + offset) % t->len]->key = 0;
			t->elems[(index + offset) % t->len]->value = NULL;
			t->size -= 1;
		}
	}
	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
			break;
		case LOCKFREEC:
			lockfreec_b = t->b;
			pthread_mutex_unlock(&lockfreec_b->locks[index % lockfreec_b->len]);
			break;
		case LINEAR:
			linear_b = t->b;
			pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
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
	linear_blob *linear_b;

	int offset = 0;

	if((index & t->mask) >= t->len) {
		return(success);
	}

	int old_len = t->len;

	switch(t->type) {
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_rdlock(&locking_b->locks[index % locking_b->len]);
			break;
		case LINEAR:
			linear_b = t->b;
			// for(offset = t->elems[index]->offset; offset < t->len; offset++) {
			for(offset = 0; offset < t->len; offset++) {
				pthread_mutex_lock(&linear_b->locks[(index + offset) % linear_b->len]);
				// unlock if the potential spot is occupied
				if(!t->elems[(index + offset) % t->len]->value || (t->elems[(index + offset) % t->len]->key != key)) {
					pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
				} else {
					success |= 1;
					break;
				}
			}
		default:
			break;
	}
	if(old_len != t->len) {
		switch(t->type) {
			case LOCKING:
				locking_b = t->b;
				pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
				break;
			case LINEAR:
				linear_b = t->b;
				pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
				break;
			default:
				break;
		}
		return(ht_contains(t, key));
	}
	if(t->type != LINEAR) {
		success |= contains_list((serial_list *) t->buckets[index], key);
	}
	switch(t->type) {
		locking_blob *locking_b;
		case LOCKING:
			locking_b = t->b;
			pthread_rwlock_unlock(&locking_b->locks[index % locking_b->len]);
			break;
		case LINEAR:
			linear_b = t->b;
			pthread_mutex_unlock(&linear_b->locks[(index + offset) % linear_b->len]);
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

void linear_element_free(linear_element *elem) {
	free((packet *) elem->value);
	free(elem);
}
