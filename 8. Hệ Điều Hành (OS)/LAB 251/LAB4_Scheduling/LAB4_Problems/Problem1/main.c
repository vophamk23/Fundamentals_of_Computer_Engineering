// ========================================
// main.c - Test Program
// ========================================

#include <stdio.h>
#include <unistd.h>
#include "bktpool.h"

// Global variables
int taskid_seed = 0;
int wrkid_cur = 0;
int bktask_sz = 0;
int wrkid_tid[MAX_WORKER];
int wrkid_busy[MAX_WORKER];
struct bktask_t *bktask = NULL;
struct bkworker_t worker[MAX_WORKER];

// Task function
int func(void *arg)
{
  int id = *((int *)arg);
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

  // Initialize arrays
  for (int i = 0; i < MAX_WORKER; i++)
  {
    wrkid_busy[i] = 0;
    wrkid_tid[i] = 0;
    worker[i].func = NULL;
    worker[i].arg = NULL;
    worker[i].bktaskid = -1;
    worker[i].wrkid = i;
  }

  // STEP 1: Initialize task pool
  ret = bktpool_init();
  if (ret != 0)
    return -1;

  // STEP 2: Create 3 tasks
  id[0] = 1;
  bktask_init(&tid[0], &func, (void *)&id[0]);

  id[1] = 2;
  bktask_init(&tid[1], &func, (void *)&id[1]);

  id[2] = 5;
  bktask_init(&tid[2], &func, (void *)&id[2]);

  // STEP 3: Task 0 - SYNCHRONOUS
  // Chạy xong mới tiếp tục
  wid[0] = bkwrk_get_worker(); // → Worker 0
  ret = bktask_assign_worker(tid[0], wid[0]);

  if (ret != 0)
    printf("assign task failed tid=%u wid=%d\n", tid[0], wid[0]);
  else
    printf("\n");

  bkwrk_dispatch_worker(wid[0]);
  usleep(100000); // Đợi Task 0 xong → Worker 0 IDLE

  // STEP 4: Task 1 - ASYNCHRONOUS
  wid[1] = bkwrk_get_worker(); // → Worker 0 (đã IDLE)
  ret = bktask_assign_worker(tid[1], wid[1]);

  if (ret != 0)
    printf("assign task failed tid=%u wid=%d\n", tid[1], wid[1]);
  else
    printf(">>>>>>>>>> Activate asynchronously\n");

  // STEP 5: Task 2 - ASYNCHRONOUS
  wid[2] = bkwrk_get_worker(); // → Worker 1 (Worker 0 BUSY)
  ret = bktask_assign_worker(tid[2], wid[2]);

  if (ret != 0)
    printf("assign task failed tid=%u wid=%d\n", tid[2], wid[2]);
  else
    printf(">>>>>>>>>> Activate asynchronously\n");

  // Dispatch cả 2 tasks
  bkwrk_dispatch_worker(wid[1]); // Worker 0
  usleep(10000);                 // Delay 10ms
  bkwrk_dispatch_worker(wid[2]); // Worker 1

  fflush(stdout);
  sleep(1); // Đợi các task xong

#ifdef STRESS_TEST
  // ===== STRESS TEST: 15 tasks =====
  int i = 0;
  for (i = 0; i < 15; i++)
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

  return 0;
}