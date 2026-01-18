// ========================================
// bktpool.h - Task Pool Header
// ========================================

#ifndef BKTPOOL_H
#define BKTPOOL_H

#include <stdlib.h>
#include <pthread.h>

// Configuration
#define MAX_WORKER 10
#define STACK_SIZE 4096
#define SIG_DISPATCH SIGUSR1

// Task structure (linked list)
struct bktask_t
{
  void (*func)(void *arg); // Hàm thực thi
  void *arg;               // Tham số truyền vào
  unsigned int bktaskid;   // ID duy nhất
  struct bktask_t *tnext;  // Con trỏ task kế tiếp
};

// Worker structure
struct bkworker_t
{
  void (*func)(void *arg); // Task hiện tại
  void *arg;               // Tham số task
  unsigned int wrkid;      // Worker ID
  unsigned int bktaskid;   // Task ID (-1 = idle)
};

// Global variables
extern int taskid_seed;            // Task ID counter
extern int wrkid_tid[MAX_WORKER];  // Worker TIDs
extern int wrkid_busy[MAX_WORKER]; // 0=IDLE, 1=BUSY
extern struct bktask_t *bktask;    // Task list head
extern int bktask_sz;              // Số lượng task
extern struct bkworker_t worker[MAX_WORKER];

// Function prototypes
int bktpool_init();
struct bktask_t *bktask_get_byid(unsigned int bktaskid);
int bktask_init(unsigned int *bktaskid, void *func, void *arg);
int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid);
void *bkwrk_worker(int *arg);
int bkwrk_create_worker();
int bkwrk_dispatch_worker(unsigned int wrkid);
int bkwrk_get_worker(); // PROBLEM 1: FIFO Scheduler

#endif