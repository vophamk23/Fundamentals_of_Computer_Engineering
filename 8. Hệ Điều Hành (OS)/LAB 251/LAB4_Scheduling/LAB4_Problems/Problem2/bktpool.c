/**
 * bktpool.c - BK Task Pool Initialization
 *
 * Module initialization cho task pool
 */

#include "bktpool.h"
#include <stdio.h>

/**
 * bktpool_init - Initialize task pool
 *
 * Khởi tạo worker pool bằng cách gọi bkwrk_create_worker()
 *
 * Return: 0 on success, -1 on failure
 */
int bktpool_init()
{
   return bkwrk_create_worker();
}