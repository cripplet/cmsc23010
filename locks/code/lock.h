#ifndef LOCKS_H
#define LOCKS_H

/* Lock types */
#define TTAS 1	// test and set
#define BACK 2	// exponential backoff lock
#define MUTX 4	// pthread mutex
#define ALCK 8	// Anderson array lock
#define CLHQ 16	// the CLH queue lock
#define MCSQ 32	// the MCS queue lock

/* Lock wrapper struct */
typedef struct lock_t {
	int type;
	// copy of the locked status
	//	FREE	0
	//	LOCKED	1
	int status;
	void *l;
} lock;

/* QNode object as defined in Herlihy */
typedef struct qnode_t {
	int *my_node;
	int *my_pred;
} qnode;

lock *init_lock(int type, void *args);
int l_lock(lock *l, void *args);
int l_try(lock *l);
int l_unlock(lock *l, void *args);

#endif
