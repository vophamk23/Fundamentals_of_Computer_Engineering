
// ========================================
// bktask.c - Task Management
// ========================================

#include "bktpool.h"
#include <stdio.h>

// Tìm task theo ID
struct bktask_t *bktask_get_byid(unsigned int bktaskid)
{
  // Validate ID
  if (bktaskid >= (unsigned int)bktask_sz)
    return NULL;

  // Traverse linked list
  struct bktask_t *ptask = bktask;
  while (ptask != NULL)
  {
    if (ptask->bktaskid == bktaskid)
      return ptask;
    ptask = ptask->tnext;
  }

  return NULL;
}

// Tạo task mới
int bktask_init(unsigned int *bktaskid, void *func, void *arg)
{
  // Validate input
  if (!bktaskid || !func)
    return -1;

  // Allocate memory
  struct bktask_t *new_task = malloc(sizeof(struct bktask_t));
  if (!new_task)
    return -1;

  // Assign ID và thêm vào đầu list
  *bktaskid = taskid_seed++;
  bktask_sz++;

  new_task->func = func;
  new_task->arg = arg;
  new_task->bktaskid = *bktaskid;
  new_task->tnext = bktask;
  bktask = new_task;

  return 0;
}