/**
 * bkshm.c - Shared Memory Management Implementation
 */

#include "bkshm.h"
#include "bktpool.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// ===== Shared Memory Names =====
#define SHM_BUSY_NAME "/bktpool_busy"
#define SHM_WORKER_NAME "/bktpool_worker"

// ===== Private Variables =====
static int *shm_wrkid_busy = NULL;
static struct bkworker_t *shm_worker = NULL;

static int shm_busy_fd = -1;
static int shm_worker_fd = -1;

/**
 * bkshm_init - Initialize shared memory
 */
int bkshm_init(void)
{
    // ===== 1. Create shared memory for wrkid_busy array =====
    shm_busy_fd = shm_open(SHM_BUSY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_busy_fd == -1)
    {
        perror("shm_open(wrkid_busy) failed");
        return -1;
    }

    // Set size
    if (ftruncate(shm_busy_fd, MAX_WORKER * sizeof(int)) == -1)
    {
        perror("ftruncate(wrkid_busy) failed");
        close(shm_busy_fd);
        shm_unlink(SHM_BUSY_NAME);
        return -1;
    }

    // Map to memory
    shm_wrkid_busy = mmap(NULL,
                          MAX_WORKER * sizeof(int),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED,
                          shm_busy_fd,
                          0);

    if (shm_wrkid_busy == MAP_FAILED)
    {
        perror("mmap(wrkid_busy) failed");
        close(shm_busy_fd);
        shm_unlink(SHM_BUSY_NAME);
        return -1;
    }

    // Initialize to 0
    memset(shm_wrkid_busy, 0, MAX_WORKER * sizeof(int));

    // ===== 2. Create shared memory for worker array =====
    shm_worker_fd = shm_open(SHM_WORKER_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_worker_fd == -1)
    {
        perror("shm_open(worker) failed");
        munmap(shm_wrkid_busy, MAX_WORKER * sizeof(int));
        close(shm_busy_fd);
        shm_unlink(SHM_BUSY_NAME);
        return -1;
    }

    // Set size
    if (ftruncate(shm_worker_fd, MAX_WORKER * sizeof(struct bkworker_t)) == -1)
    {
        perror("ftruncate(worker) failed");
        munmap(shm_wrkid_busy, MAX_WORKER * sizeof(int));
        close(shm_busy_fd);
        close(shm_worker_fd);
        shm_unlink(SHM_BUSY_NAME);
        shm_unlink(SHM_WORKER_NAME);
        return -1;
    }

    // Map to memory
    shm_worker = mmap(NULL,
                      MAX_WORKER * sizeof(struct bkworker_t),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      shm_worker_fd,
                      0);

    if (shm_worker == MAP_FAILED)
    {
        perror("mmap(worker) failed");
        munmap(shm_wrkid_busy, MAX_WORKER * sizeof(int));
        close(shm_busy_fd);
        close(shm_worker_fd);
        shm_unlink(SHM_BUSY_NAME);
        shm_unlink(SHM_WORKER_NAME);
        return -1;
    }

    // Initialize worker array
    for (int i = 0; i < MAX_WORKER; i++)
    {
        shm_worker[i].func = NULL;
        shm_worker[i].arg = NULL;
        shm_worker[i].bktaskid = -1;
        shm_worker[i].wrkid = i;
    }

    return 0;
}

/**
 * bkshm_cleanup - Cleanup shared memory
 */
void bkshm_cleanup(void)
{
    // Unmap memory
    if (shm_wrkid_busy != NULL)
    {
        munmap(shm_wrkid_busy, MAX_WORKER * sizeof(int));
        shm_wrkid_busy = NULL;
    }

    if (shm_worker != NULL)
    {
        munmap(shm_worker, MAX_WORKER * sizeof(struct bkworker_t));
        shm_worker = NULL;
    }

    // Close file descriptors
    if (shm_busy_fd != -1)
    {
        close(shm_busy_fd);
        shm_busy_fd = -1;
    }

    if (shm_worker_fd != -1)
    {
        close(shm_worker_fd);
        shm_worker_fd = -1;
    }

    // Unlink shared memory
    shm_unlink(SHM_BUSY_NAME);
    shm_unlink(SHM_WORKER_NAME);
}

/**
 * bkshm_get_busy_array - Get busy array pointer
 */
int *bkshm_get_busy_array(void)
{
    return shm_wrkid_busy;
}

/**
 * bkshm_get_worker_array - Get worker array pointer
 */
struct bkworker_t *bkshm_get_worker_array(void)
{
    return shm_worker;
}