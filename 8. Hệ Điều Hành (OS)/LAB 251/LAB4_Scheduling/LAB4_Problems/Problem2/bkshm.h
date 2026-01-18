/**
 * bkshm.h - Shared Memory Management
 *
 * Quản lý shared memory cho fork-based worker pool
 */

#ifndef BKSHM_H
#define BKSHM_H

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_WORKER 10

// Forward declaration
struct bkworker_t;

/**
 * bkshm_init - Initialize shared memory segments
 *
 * Tạo 2 shared memory regions:
 *   1. wrkid_busy array
 *   2. worker array
 *
 * Return: 0 on success, -1 on failure
 */
int bkshm_init(void);

/**
 * bkshm_cleanup - Cleanup shared memory
 *
 * Unmap và unlink shared memory segments
 */
void bkshm_cleanup(void);

/**
 * bkshm_get_busy_array - Get pointer to shared busy array
 *
 * Return: Pointer to wrkid_busy array in shared memory
 */
int *bkshm_get_busy_array(void);

/**
 * bkshm_get_worker_array - Get pointer to shared worker array
 *
 * Return: Pointer to worker array in shared memory
 */
struct bkworker_t *bkshm_get_worker_array(void);

#endif /* BKSHM_H */