/**
 * bkfj.h - Fork-Join Framework Header
 */

#ifndef BKFJ_H
#define BKFJ_H

#include <pthread.h>
#include <semaphore.h>

#define MAX_FJ_WORKERS 8

typedef enum
{
    FJ_TASK_PENDING,
    FJ_TASK_RUNNING,
    FJ_TASK_COMPLETED
} fj_task_status_t;

typedef struct fj_task
{
    unsigned int task_id;
    void *(*func)(void *);
    void *arg;
    void *result;
    fj_task_status_t status;
    struct fj_task *next;
} fj_task_t;

typedef struct
{
    pthread_t workers[MAX_FJ_WORKERS];
    int num_workers;

    fj_task_t *task_queue_head;
    fj_task_t *task_queue_tail;
    pthread_mutex_t queue_mutex;
    sem_t task_available;

    fj_task_t *completed_head;
    pthread_mutex_t completed_mutex;

    int active_tasks;
    pthread_mutex_t count_mutex;
    pthread_cond_t all_done_cond;

    int shutdown;
} fj_pool_t;

fj_pool_t *fj_pool_init(int num_workers);
void fj_pool_destroy(fj_pool_t *pool);
int fj_fork(fj_pool_t *pool, void *(*func)(void *), void *arg);
void fj_join(fj_pool_t *pool);
void *fj_get_result(fj_pool_t *pool, unsigned int task_id);
void fj_free_result(fj_pool_t *pool, unsigned int task_id);

#endif /* BKFJ_H */