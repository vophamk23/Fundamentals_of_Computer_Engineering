/**
 * bkfj.c - Improved Fork-Join Framework Implementation
 * Fixes: memory leaks, thread safety, race conditions
 */

#include "bkfj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Thread-safe task ID counter
static pthread_mutex_t task_id_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned int task_id_counter = 0;

static unsigned int get_next_task_id(void)
{
    pthread_mutex_lock(&task_id_mutex);
    unsigned int id = task_id_counter++;
    pthread_mutex_unlock(&task_id_mutex);
    return id;
}

static void *fj_worker_thread(void *arg)
{
    fj_pool_t *pool = (fj_pool_t *)arg;

    while (1)
    {
        sem_wait(&pool->task_available);

        pthread_mutex_lock(&pool->queue_mutex);
        if (pool->shutdown && pool->task_queue_head == NULL)
        {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }

        fj_task_t *task = pool->task_queue_head;
        if (task == NULL)
        {
            pthread_mutex_unlock(&pool->queue_mutex);
            continue;
        }

        pool->task_queue_head = task->next;
        if (pool->task_queue_head == NULL)
            pool->task_queue_tail = NULL;
        pthread_mutex_unlock(&pool->queue_mutex);

        // Execute task
        task->status = FJ_TASK_RUNNING;
        void *result = task->func(task->arg);

        // Store result atomically
        task->result = result;
        __sync_synchronize(); // Memory barrier
        task->status = FJ_TASK_COMPLETED;

        // Move to completed list
        pthread_mutex_lock(&pool->completed_mutex);
        task->next = pool->completed_head;
        pool->completed_head = task;
        pthread_mutex_unlock(&pool->completed_mutex);

        // Update counter
        pthread_mutex_lock(&pool->count_mutex);
        pool->active_tasks--;
        if (pool->active_tasks == 0)
            pthread_cond_broadcast(&pool->all_done_cond);
        pthread_mutex_unlock(&pool->count_mutex);
    }

    return NULL;
}

fj_pool_t *fj_pool_init(int num_workers)
{
    if (num_workers <= 0 || num_workers > MAX_FJ_WORKERS)
        return NULL;

    fj_pool_t *pool = malloc(sizeof(fj_pool_t));
    if (!pool)
        return NULL;

    memset(pool, 0, sizeof(fj_pool_t));
    pool->num_workers = num_workers;

    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_mutex_init(&pool->completed_mutex, NULL);
    pthread_mutex_init(&pool->count_mutex, NULL);
    pthread_cond_init(&pool->all_done_cond, NULL);
    sem_init(&pool->task_available, 0, 0);

    // Create worker threads
    for (int i = 0; i < num_workers; i++)
    {
        if (pthread_create(&pool->workers[i], NULL, fj_worker_thread, pool) != 0)
        {
            // Cleanup on failure
            pool->num_workers = i; // Only join created threads
            fj_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

void fj_pool_destroy(fj_pool_t *pool)
{
    if (!pool)
        return;

    // Signal shutdown
    pool->shutdown = 1;

    // Wake up all workers
    for (int i = 0; i < pool->num_workers; i++)
        sem_post(&pool->task_available);

    // Wait for workers to finish
    for (int i = 0; i < pool->num_workers; i++)
        pthread_join(pool->workers[i], NULL);

    // Free pending tasks
    fj_task_t *task = pool->task_queue_head;
    while (task)
    {
        fj_task_t *next = task->next;
        free(task);
        task = next;
    }

    // Free completed tasks (including results)
    task = pool->completed_head;
    while (task)
    {
        fj_task_t *next = task->next;
        if (task->result)
            free(task->result);
        free(task);
        task = next;
    }

    // Destroy synchronization primitives
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_mutex_destroy(&pool->completed_mutex);
    pthread_mutex_destroy(&pool->count_mutex);
    pthread_cond_destroy(&pool->all_done_cond);
    sem_destroy(&pool->task_available);

    free(pool);
}

int fj_fork(fj_pool_t *pool, void *(*func)(void *), void *arg)
{
    if (!pool || !func || pool->shutdown)
        return -1;

    fj_task_t *task = malloc(sizeof(fj_task_t));
    if (!task)
        return -1;

    task->task_id = get_next_task_id(); // Thread-safe counter
    task->func = func;
    task->arg = arg;
    task->result = NULL;
    task->status = FJ_TASK_PENDING;
    task->next = NULL;

    // Increment active task counter
    pthread_mutex_lock(&pool->count_mutex);
    pool->active_tasks++;
    pthread_mutex_unlock(&pool->count_mutex);

    // Add to queue
    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->task_queue_tail == NULL)
    {
        pool->task_queue_head = pool->task_queue_tail = task;
    }
    else
    {
        pool->task_queue_tail->next = task;
        pool->task_queue_tail = task;
    }
    pthread_mutex_unlock(&pool->queue_mutex);

    // Notify worker
    sem_post(&pool->task_available);

    return task->task_id;
}

void fj_join(fj_pool_t *pool)
{
    if (!pool)
        return;

    pthread_mutex_lock(&pool->count_mutex);
    while (pool->active_tasks > 0)
    {
        pthread_cond_wait(&pool->all_done_cond, &pool->count_mutex);
    }
    pthread_mutex_unlock(&pool->count_mutex);
}

void *fj_get_result(fj_pool_t *pool, unsigned int task_id)
{
    if (!pool)
        return NULL;

    pthread_mutex_lock(&pool->completed_mutex);

    fj_task_t *task = pool->completed_head;
    while (task)
    {
        if (task->task_id == task_id)
        {
            void *result = task->result;
            pthread_mutex_unlock(&pool->completed_mutex);
            return result;
        }
        task = task->next;
    }

    pthread_mutex_unlock(&pool->completed_mutex);
    return NULL;
}

void fj_free_result(fj_pool_t *pool, unsigned int task_id)
{
    if (!pool)
        return;

    pthread_mutex_lock(&pool->completed_mutex);

    fj_task_t **curr = &pool->completed_head;
    while (*curr)
    {
        if ((*curr)->task_id == task_id)
        {
            fj_task_t *to_free = *curr;
            *curr = (*curr)->next;

            // Free result if exists
            if (to_free->result)
                free(to_free->result);
            free(to_free);

            pthread_mutex_unlock(&pool->completed_mutex);
            return;
        }
        curr = &(*curr)->next;
    }

    pthread_mutex_unlock(&pool->completed_mutex);
}

// NEW: Get result and auto-free task
void *fj_get_and_free_result(fj_pool_t *pool, unsigned int task_id)
{
    if (!pool)
        return NULL;

    pthread_mutex_lock(&pool->completed_mutex);

    fj_task_t **curr = &pool->completed_head;
    while (*curr)
    {
        if ((*curr)->task_id == task_id)
        {
            fj_task_t *task = *curr;
            *curr = (*curr)->next;

            void *result = task->result;
            free(task); // Free task struct but return result

            pthread_mutex_unlock(&pool->completed_mutex);
            return result;
        }
        curr = &(*curr)->next;
    }

    pthread_mutex_unlock(&pool->completed_mutex);
    return NULL;
}

// NEW: Get all results
int fj_get_all_results(fj_pool_t *pool, void ***results, int *count)
{
    if (!pool || !results || !count)
        return -1;

    pthread_mutex_lock(&pool->completed_mutex);

    // Count completed tasks
    int n = 0;
    fj_task_t *task = pool->completed_head;
    while (task)
    {
        n++;
        task = task->next;
    }

    if (n == 0)
    {
        *results = NULL;
        *count = 0;
        pthread_mutex_unlock(&pool->completed_mutex);
        return 0;
    }

    // Allocate result array
    *results = malloc(n * sizeof(void *));
    if (!*results)
    {
        pthread_mutex_unlock(&pool->completed_mutex);
        return -1;
    }

    // Copy results
    task = pool->completed_head;
    for (int i = 0; i < n; i++)
    {
        (*results)[i] = task->result;
        task = task->next;
    }

    *count = n;
    pthread_mutex_unlock(&pool->completed_mutex);
    return 0;
}