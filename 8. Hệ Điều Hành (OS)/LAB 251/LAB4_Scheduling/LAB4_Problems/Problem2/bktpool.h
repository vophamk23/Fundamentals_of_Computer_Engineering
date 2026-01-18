#ifndef BKTPOOL_H
#define BKTPOOL_H

#include <stdlib.h>
#include <pthread.h>

// Configuration
#define MAX_WORKER 10
#define STACK_SIZE 4096
#define SIG_DISPATCH SIGUSR1

// Task structure
struct bktask_t
{
  void (*func)(void *arg);
  void *arg;
  unsigned int bktaskid;
  struct bktask_t *tnext;
};

// Worker structure
struct bkworker_t
{
  void (*func)(void *arg);
  void *arg;
  unsigned int wrkid;
  unsigned int bktaskid;
};

// ===== Global Variables - Conditional =====
extern int taskid_seed;
extern int wrkid_tid[MAX_WORKER];
extern int bktask_sz;
extern struct bktask_t *bktask;

#ifdef WORK_THREAD
// Thread version: normal arrays
extern int wrkid_busy[MAX_WORKER];
extern struct bkworker_t worker[MAX_WORKER];
#else
// Fork version: pointers to shared memory
extern int *wrkid_busy;
extern struct bkworker_t *worker;
#endif

// Function prototypes
int bktpool_init(void);
struct bktask_t *bktask_get_byid(unsigned int bktaskid);
int bktask_init(unsigned int *bktaskid, void *func, void *arg);
int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid);
void *bkwrk_worker(int *arg);
int bkwrk_create_worker(void);
int bkwrk_dispatch_worker(unsigned int wrkid);
int bkwrk_get_worker(void);

#endif