/*
 gcc -pthread -std=c11 -o detector detector.c
 ./detector
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdatomic.h>

#define MAX_WORKERS 3       // Số worker threads
#define CHECK_INTERVAL 3    // Kiểm tra mỗi 2 giây
#define FAILURE_THRESHOLD 4 // Tắt hệ thống sau 4 lần lỗi
#define MAX_TASK_TIME 8     // Worker bị stuck nếu > 8 giây không phản hồi

// Trạng thái worker
typedef struct
{
    int id;
    atomic_bool is_alive;  // Còn sống?
    time_t last_heartbeat; // Lần gửi heartbeat cuối
    int task_count;        // Số task đã làm
    bool is_stuck;         // Bị stuck?
} worker_state_t;

worker_state_t workers[MAX_WORKERS];
pthread_mutex_t lock;
atomic_bool system_shutdown = false; // Cờ tắt hệ thống
int failure_count = 0;               // Đếm số lần phát hiện lỗi

// Thống kê
int total_recoveries = 0;
int total_checks = 0;

// ─────────────────────────────────────────────────────────────────
//  KIỂM TRA TÌNH TRẠNG HỆ THỐNG
// ─────────────────────────────────────────────────────────────────
int is_safe()
{
    int unsafe_count = 0;
    time_t current_time = time(NULL);

    printf("\n[CHECK] System health status:\n");
    total_checks++;

    for (int i = 0; i < MAX_WORKERS; i++)
    {
        // Worker chết
        if (!atomic_load(&workers[i].is_alive))
        {
            printf("  Worker %d: DEAD\n", i);
            unsafe_count++;
            continue;
        }

        // Worker bị stuck (quá lâu không heartbeat)
        time_t time_since_heartbeat = current_time - workers[i].last_heartbeat;
        if (time_since_heartbeat > MAX_TASK_TIME)
        {
            printf("  Worker %d: STUCK (%ld sec)\n", i, time_since_heartbeat);
            workers[i].is_stuck = true;
            unsafe_count++;
        }
        else
        {
            printf("  Worker %d: OK\n", i);
            workers[i].is_stuck = false;
        }
    }

    // Hệ thống không an toàn nếu có lỗi
    if (unsafe_count > 0)
    {
        printf("Result: UNSAFE (%d issues)\n", unsafe_count);
        return -1;
    }

    printf("Result: SAFE\n");
    return 0;
}

// ─────────────────────────────────────────────────────────────────
//  PHỤC HỒI WORKER
// ─────────────────────────────────────────────────────────────────
void recover_worker(int worker_id)
{
    printf("  [RECOVERY] Attempting to recover Worker %d...\n", worker_id);

    // Reset worker bị stuck
    if (workers[worker_id].is_stuck)
    {
        printf("    → Resetting stuck worker\n");
        workers[worker_id].last_heartbeat = time(NULL);
        workers[worker_id].is_stuck = false;
        workers[worker_id].task_count = 0;
        total_recoveries++;
    }
    // Khởi động lại worker chết
    else if (!atomic_load(&workers[worker_id].is_alive))
    {
        printf("    → Restarting dead worker\n");
        atomic_store(&workers[worker_id].is_alive, true);
        workers[worker_id].last_heartbeat = time(NULL);
        workers[worker_id].task_count = 0;
        total_recoveries++;
    }

    printf("  [RECOVERY] Worker %d recovered successfully\n", worker_id);
}

// ─────────────────────────────────────────────────────────────────
//  DETECTOR THREAD - Kiểm tra định kỳ
// ─────────────────────────────────────────────────────────────────
void *periodical_detector(void *arg)
{
    printf("[DETECTOR] Starting periodic health checker (interval: %d sec)\n",
           CHECK_INTERVAL);

    while (1)
    {
        sleep(CHECK_INTERVAL); // Kiểm tra mỗi 2 giây

        if (atomic_load(&system_shutdown))
        {
            printf("[DETECTOR] Shutdown signal received, exiting...\n");
            break;
        }

        pthread_mutex_lock(&lock);

        // Nếu phát hiện lỗi
        if (!is_safe())
        {
            printf("\n[DETECTOR] ⚠️  ANOMALY DETECTED! Initiating recovery...\n");
            failure_count++;

            // Phục hồi tất cả worker có vấn đề
            for (int i = 0; i < MAX_WORKERS; i++)
            {
                if (workers[i].is_stuck || !atomic_load(&workers[i].is_alive))
                {
                    recover_worker(i);
                }
            }

            // Quá nhiều lỗi → tắt hệ thống
            if (failure_count >= FAILURE_THRESHOLD)
            {
                printf("\n[DETECTOR] ❌ CRITICAL FAILURE! System failed %d times\n",
                       failure_count);
                printf("[DETECTOR] Initiating emergency shutdown...\n");
                atomic_store(&system_shutdown, true);
                pthread_mutex_unlock(&lock);
                break;
            }
        }
        else
        {
            failure_count = 0; // Reset nếu hệ thống OK
        }

        pthread_mutex_unlock(&lock);
    }

    printf("[DETECTOR] Periodic detector terminated\n");
    return NULL;
}

// ─────────────────────────────────────────────────────────────────
//  WORKER THREAD - Làm việc và gửi heartbeat
// ─────────────────────────────────────────────────────────────────
void *worker_thread(void *arg)
{
    int id = *(int *)arg;
    printf("[Worker %d] Started\n", id);

    while (!atomic_load(&system_shutdown))
    {
        // Nếu worker chết → chờ được phục hồi
        if (!atomic_load(&workers[id].is_alive))
        {
            while (!atomic_load(&workers[id].is_alive) &&
                   !atomic_load(&system_shutdown))
            {
                sleep(1);
            }

            if (atomic_load(&system_shutdown))
                break;

            printf("[Worker %d] ✨ Resurrected by recovery system!\n", id);
            continue;
        }

        // Làm task và gửi heartbeat
        pthread_mutex_lock(&lock);
        workers[id].task_count++;
        workers[id].last_heartbeat = time(NULL);
        printf("[Worker %d] Task #%d completed (heartbeat sent)\n",
               id, workers[id].task_count);
        pthread_mutex_unlock(&lock);

        int work_time = 1 + (rand() % 5);

        // Mô phỏng lỗi
        if (id == 2 && (rand() % 10) < 2) // Worker 2: 20% bị stuck
        {
            printf("[Worker %d] 💤 Simulating STUCK condition...\n", id);
            sleep(15);
        }
        else if (id == 4 && (rand() % 10) < 1) // Worker 4: 10% chết
        {
            printf("[Worker %d] 💀 Simulating DEATH...\n", id);
            atomic_store(&workers[id].is_alive, false);
        }
        else
        {
            sleep(work_time);
        }
    }

    printf("[Worker %d] Shutdown\n", id);
    return NULL;
}

// ─────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────
int main()
{
    pthread_t detector_thread;
    pthread_t worker_threads[MAX_WORKERS];
    int worker_ids[MAX_WORKERS];

    printf("═══════════════════════════════════════════════════\n");
    printf("   PERIODIC HEALTH CHECK & RECOVERY SYSTEM\n");
    printf("═══════════════════════════════════════════════════\n\n");

    pthread_mutex_init(&lock, NULL);
    srand(time(NULL));

    // Khởi tạo workers
    for (int i = 0; i < MAX_WORKERS; i++)
    {
        workers[i].id = i;
        atomic_store(&workers[i].is_alive, true);
        workers[i].last_heartbeat = time(NULL);
        workers[i].task_count = 0;
        workers[i].is_stuck = false;
    }

    // Tạo detector thread
    printf("[MAIN] Starting health detector...\n");
    pthread_create(&detector_thread, NULL, periodical_detector, NULL);

    // Tạo worker threads
    printf("[MAIN] Starting %d worker threads...\n\n", MAX_WORKERS);
    for (int i = 0; i < MAX_WORKERS; i++)
    {
        worker_ids[i] = i;
        pthread_create(&worker_threads[i], NULL, worker_thread, &worker_ids[i]);
    }

    printf("[MAIN] System running... (will auto-shutdown in 30 sec)\n\n");
    sleep(30); // Chạy 30 giây

    // Tắt hệ thống
    if (!atomic_load(&system_shutdown))
    {
        printf("\n[MAIN] Initiating graceful shutdown...\n");
        atomic_store(&system_shutdown, true);
    }

    pthread_join(detector_thread, NULL);
    for (int i = 0; i < MAX_WORKERS; i++)
    {
        pthread_join(worker_threads[i], NULL);
    }

    // In thống kê
    printf("\n═══════════════════════════════════════════════════\n");
    printf("   SYSTEM STATISTICS\n");
    printf("═══════════════════════════════════════════════════\n");
    printf("Total health checks: %d\n", total_checks);
    printf("Total recoveries: %d\n", total_recoveries);
    printf("Final failure count: %d\n", failure_count);
    printf("═══════════════════════════════════════════════════\n");

    pthread_mutex_destroy(&lock);
    return 0;
}