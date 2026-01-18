/**
 * bkwrk.c - Worker Management Module
 *
 * Quản lý workers: tạo, dispatch, scheduler
 *
 * ===== PROBLEM 1: FIFO SCHEDULER IMPLEMENTATION =====
 */

#include "bktpool.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>

// #define _GNU_SOURCE
#include <sched.h>       // For clone() and CLONE_* flags
#include <sys/syscall.h> // For SYS_tkill
#include <unistd.h>

// #define DEBUG
// #define INFO
#define WORK_THREAD

/**
 * bkwrk_worker - Worker thread main loop
 *
 * @arg: Worker ID (as void pointer)
 *
 * Flow:
 *   1. Setup signal mask (block SIGUSR1, SIGQUIT)
 *   2. Loop:
 *      a. sigwait() - chờ signal
 *      b. Execute task nếu có (wrk->func != NULL)
 *      c. Set status back to IDLE
 *
 * Return: NULL (never returns in normal operation)
 */
void *bkwrk_worker(int *arg)
{
  // ===== FIX: Disable buffering cho fork version =====
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  sigset_t set;
  int sig;
  int s;
  int i = *((int *)arg); // Worker ID
  struct bkworker_t *wrk = &worker[i];

  /* Setup signal mask for waking up */
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGQUIT);

#ifdef DEBUG
  fprintf(stderr, "worker %i start living tid %d \n", i, getpid());
  fflush(stderr);
#endif

  while (1)
  {
    /* Wait for signal */
    s = sigwait(&set, &sig);
    if (s != 0)
      continue;

#ifdef INFO
    fprintf(stderr, "worker wake %d up\n", i);
    fflush(stderr); // ← THÊM fflush
#endif

    /* Execute task if assigned */
    if (wrk->func != NULL)
      wrk->func(wrk->arg);

    /* Mark as DONE - back to IDLE state */
    wrkid_busy[i] = 0;
    worker[i].func = NULL;
    worker[i].arg = NULL;
    worker[i].bktaskid = -1;
  }

  return 0;
}

/**
 * bktask_assign_worker - Assign task to worker
 *
 * @bktaskid: Task ID to assign
 * @wrkid: Worker ID to assign to
 *
 * Steps:
 *   1. Validate worker ID
 *   2. Get task by ID
 *   3. Mark worker as BUSY
 *   4. Copy task info to worker
 *
 * Return: 0 on success, -1 on failure
 */
int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid)
{
  // Validate
  if (wrkid >= MAX_WORKER)
    return -1;

  struct bktask_t *tsk = bktask_get_byid(bktaskid);
  if (!tsk)
    return -1;

  // Mark worker BUSY và copy task info
  wrkid_busy[wrkid] = 1;
  worker[wrkid].func = tsk->func;
  worker[wrkid].arg = tsk->arg;
  worker[wrkid].bktaskid = bktaskid;

  printf("Assign tsk %d wrk %d ", tsk->bktaskid, wrkid);
  return 0;
}

/**
 * bkwrk_create_worker - Create worker pool
 *
 * Tạo MAX_WORKER workers sử dụng clone() system call
 *
 * CRITICAL FIX: Race condition với worker IDs
 *
 * Steps for each worker:
 *   1. Allocate stack
 *   2. Setup signal mask
 *   3. Call clone() with CLONE_VM | CLONE_FILES
 *   4. Store worker TID
 *   5. Initialize worker state
 *
 * Return: 0 on success, -1 on failure
 */
int bkwrk_create_worker()
{
  // CRITICAL: Dùng static array để tránh race condition
  static unsigned int worker_ids[MAX_WORKER];

  for (unsigned int i = 0; i < MAX_WORKER; i++)
  {
    worker_ids[i] = i;

#ifdef WORK_THREAD
    // ===== THREAD VERSION (clone) =====

    // Allocate stack cho worker
    void **child_stack = (void **)malloc(STACK_SIZE);
    if (!child_stack)
      return -1;

    // Setup signal mask
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // Stack grows down → bắt đầu từ top
    void *stack_top = child_stack + STACK_SIZE;

    // Clone worker thread
    wrkid_tid[i] = clone(&bkwrk_worker, stack_top,
                         CLONE_VM | CLONE_FILES,
                         (void *)&worker_ids[i]);

    if (wrkid_tid[i] == -1)
    {
      perror("clone failed");
      free(child_stack);
      return -1;
    }

    // Initialize worker state
    wrkid_busy[i] = 0;
    worker[i].func = NULL;
    worker[i].arg = NULL;
    worker[i].bktaskid = -1;
    worker[i].wrkid = i;

    fprintf(stderr, "bkwrk_create_worker got worker %u\n", wrkid_tid[i]);
    usleep(1000); // 1ms delay

#else
    /* ===== PROBLEM 2: FORK VERSION ===== */
    /* TODO: Implement fork version of create worker */

#endif
  }

  return 0;
}

// ========================================
// PROBLEM 1: FIFO SCHEDULER
// ========================================
int bkwrk_get_worker()
{
  // Duyệt từ worker 0 → MAX_WORKER-1
  for (int i = 0; i < MAX_WORKER; i++)
  {
    if (wrkid_busy[i] == 0) // Tìm worker IDLE đầu tiên
      return i;
  }
  return -1; // Không có worker rảnh
}

/**
 * bkwrk_dispatch_worker - Dispatch worker to execute task
 * Send SIGUSR1 signal to wake up worker from sigwait()
 *
 * Return: 0 on success, -1 on failure
 */
int bkwrk_dispatch_worker(unsigned int wrkid)
{
  // FIXED: Validate worker ID
  if (wrkid >= MAX_WORKER)
  {
    fprintf(stderr, "ERROR: Invalid worker ID %u\n", wrkid);
    return -1;
  }

#ifdef WORK_THREAD
  unsigned int tid = wrkid_tid[wrkid];

  /* Check if worker has valid task */
  if (worker[wrkid].func == NULL)
  {
    fprintf(stderr, "WARNING: Worker %u has no task assigned\n", wrkid);
    return -1;
  }

#ifdef DEBUG
  fprintf(stderr, "brkwrk dispatch wrkid %d - send signal %u \n", wrkid, tid);
#endif

  // Send SIGUSR1 signal to wake up worker
  int ret = syscall(SYS_tkill, tid, SIG_DISPATCH);

  // Check signal failure
  if (ret != 0)
  {
    perror("tkill failed");
    return -1;
  }

#else
  /* ===== PROBLEM 2: FORK VERSION ===== */
  /* TODO: Implement fork version to signal worker process here */

#endif

  return 0;
}