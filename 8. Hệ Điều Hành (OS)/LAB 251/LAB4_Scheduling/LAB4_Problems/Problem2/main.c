#include <stdio.h>
#include <unistd.h>
#include <stdint.h> // For intptr_t
#include "bktpool.h"

#ifndef WORK_THREAD
#include "bkshm.h" // Chỉ include khi fork version
#endif

// ===== Global Variables =====
int taskid_seed = 0;
int bktask_sz = 0;
int wrkid_tid[MAX_WORKER];
struct bktask_t *bktask = NULL;

#ifdef WORK_THREAD
// Thread version: static arrays
int wrkid_busy[MAX_WORKER];
struct bkworker_t worker[MAX_WORKER];
#else
// Fork version: pointers (will point to shared memory)
int *wrkid_busy = NULL;
struct bkworker_t *worker = NULL;
#endif

// Task function
// int func(void *arg)
// {
//   int id = *((int *)arg);
//   printf("Task func - Hello from %d\n", id);
//   fflush(stdout);
//   return 0;
// }

// Task function - FIXED
int func(void *arg)
{
  // Cast pointer back to integer value
  int id = (int)(intptr_t)arg;
  printf("Task func - Hello from %d\n", id);
  fflush(stdout);
  return 0;
}

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  unsigned int tid[15];
  int wid[15];
  int id[15];
  int ret;

#ifdef WORK_THREAD
  // ===== Thread Version: Initialize arrays =====
  for (int i = 0; i < MAX_WORKER; i++)
  {
    wrkid_busy[i] = 0;
    wrkid_tid[i] = 0;
    worker[i].func = NULL;
    worker[i].arg = NULL;
    worker[i].bktaskid = -1;
    worker[i].wrkid = i;
  }
#else
  // ===== Fork Version: Setup shared memory =====
  if (bkshm_init() != 0)
  {
    return -1;
  }

  // Get pointers to shared memory
  wrkid_busy = bkshm_get_busy_array();
  worker = bkshm_get_worker_array();

  // Initialize TID array (not shared)
  for (int i = 0; i < MAX_WORKER; i++)
  {
    wrkid_tid[i] = 0;
  }
#endif

  // ===== STEP 1: Initialize task pool =====
  ret = bktpool_init();
  if (ret != 0)
  {
#ifndef WORK_THREAD
    bkshm_cleanup();
#endif
    return -1;
  }

  // ===== STEP 2: Create 3 tasks =====
  // id[0] = 1;
  // bktask_init(&tid[0], &func, (void *)&id[0]);

  // id[1] = 2;
  // bktask_init(&tid[1], &func, (void *)&id[1]);

  // id[2] = 5;
  // bktask_init(&tid[2], &func, (void *)&id[2]);

  bktask_init(&tid[0], &func, (void *)(intptr_t)1);
  bktask_init(&tid[1], &func, (void *)(intptr_t)2);
  bktask_init(&tid[2], &func, (void *)(intptr_t)5);

  // ===== STEP 3: Task 0 - SYNCHRONOUS =====
  wid[0] = bkwrk_get_worker();
  ret = bktask_assign_worker(tid[0], wid[0]);

  if (ret != 0)
    printf("assign task failed tid=%u wid=%d\n", tid[0], wid[0]);
  else
    printf("\n");

  bkwrk_dispatch_worker(wid[0]);
  usleep(100000);

  // ===== STEP 4: Task 1 - ASYNCHRONOUS =====
  wid[1] = bkwrk_get_worker();
  ret = bktask_assign_worker(tid[1], wid[1]);

  if (ret != 0)
    printf("assign task failed tid=%u wid=%d\n", tid[1], wid[1]);
  else
    printf(">>>>>>>>>> Activate asynchronously\n");

  // ===== STEP 5: Task 2 - ASYNCHRONOUS =====
  wid[2] = bkwrk_get_worker();
  ret = bktask_assign_worker(tid[2], wid[2]);

  if (ret != 0)
    printf("assign task failed tid=%u wid=%d\n", tid[2], wid[2]);
  else
    printf(">>>>>>>>>> Activate asynchronously\n");

  bkwrk_dispatch_worker(wid[1]);
  usleep(10000); // 10ms delay
  bkwrk_dispatch_worker(wid[2]);

  fflush(stdout);
  sleep(1);

#ifdef STRESS_TEST
  // ===== STRESS TEST: 15 tasks =====
  for (int i = 0; i < 15; i++)
  {
    id[i] = i;
    bktask_init(&tid[i], &func, (void *)&id[i]);

    wid[i] = bkwrk_get_worker();
    ret = bktask_assign_worker(tid[i], wid[i]);

    if (ret != 0)
      printf("assign task failed tid=%u wid=%d\n", tid[i], wid[i]);

    bkwrk_dispatch_worker(wid[i]);
  }
  sleep(3);
#endif

#ifndef WORK_THREAD
  // ===== Cleanup shared memory =====
  bkshm_cleanup();
#endif

  return 0;
}