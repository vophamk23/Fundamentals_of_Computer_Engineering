/*
gcc -pthread -o resource_async resource_async.c
./resource_async
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_RESOURCES 10 // Tổng số tài nguyên
#define NUM_PROCESSES 5  // Số process yêu cầu tài nguyên

// Cấu trúc yêu cầu của process
typedef struct
{
    int id;                  // ID process
    int requested_resources; // Số tài nguyên cần
    void (*callback)(int);   // Hàm callback khi được cấp phát
} process_request_t;

// Tài nguyên toàn cục
int available_resources = NUM_RESOURCES; // Tài nguyên còn lại
pthread_mutex_t resource_lock;           // Bảo vệ tài nguyên
pthread_cond_t resource_cond;            // CV cho process chờ

// ============================================================
// CALLBACK - Được gọi khi process nhận tài nguyên thành công
// ============================================================
void resource_callback(int process_id)
{
    printf("    [Callback] Process %d received resources successfully!\n", process_id);
}

// ============================================================
// RESOURCE MANAGER - Xử lý yêu cầu cấp phát tài nguyên
// ============================================================
void *resource_man(void *arg)
{
    process_request_t *request = (process_request_t *)arg;

    printf("[Process %d] Requesting %d resources...\n",
           request->id, request->requested_resources);

    pthread_mutex_lock(&resource_lock);

    // Chờ nếu không đủ tài nguyên
    while (request->requested_resources > available_resources)
    {
        printf("[Process %d] Waiting for resources (need %d, available %d)\n",
               request->id, request->requested_resources, available_resources);
        pthread_cond_wait(&resource_cond, &resource_lock);
    }

    // Cấp phát tài nguyên
    available_resources -= request->requested_resources;
    printf("[Process %d] Allocated %d resources (remaining: %d)\n",
           request->id, request->requested_resources, available_resources);

    // Gọi callback để thông báo thành công
    request->callback(request->id);

    pthread_mutex_unlock(&resource_lock);

    // Mô phỏng process làm việc với tài nguyên
    printf("[Process %d] Working with resources...\n", request->id);
    sleep(2);

    // Trả lại tài nguyên sau khi xong việc
    pthread_mutex_lock(&resource_lock);

    available_resources += request->requested_resources;
    printf("[Process %d] Released %d resources (remaining: %d)\n",
           request->id, request->requested_resources, available_resources);

    // Đánh thức các process đang chờ
    pthread_cond_broadcast(&resource_cond);

    pthread_mutex_unlock(&resource_lock);

    return NULL;
}

// ============================================================
// MAIN
// ============================================================
int main()
{
    pthread_t threads[NUM_PROCESSES];
    process_request_t requests[NUM_PROCESSES];

    printf("==============================================\n");
    printf("  ASYNCHRONOUS RESOURCE ALLOCATION SYSTEM\n");
    printf("==============================================\n");
    printf("Total resources: %d\n", NUM_RESOURCES);
    printf("Number of processes: %d\n\n", NUM_PROCESSES);

    // Khởi tạo mutex và condition variable
    pthread_mutex_init(&resource_lock, NULL);
    pthread_cond_init(&resource_cond, NULL);

    // Định nghĩa yêu cầu tài nguyên của từng process
    int resource_requests[] = {3, 5, 2, 4, 3};

    for (int i = 0; i < NUM_PROCESSES; i++)
    {
        requests[i].id = i;
        requests[i].requested_resources = resource_requests[i];
        requests[i].callback = resource_callback;

        printf("Process %d will request %d resources\n",
               i, requests[i].requested_resources);
    }

    printf("\n--- Starting asynchronous requests ---\n\n");

    // Tạo threads (bất đồng bộ)
    for (int i = 0; i < NUM_PROCESSES; i++)
    {
        pthread_create(&threads[i], NULL, resource_man, &requests[i]);
        usleep(100000); // Trễ 100ms giữa các request
    }

    printf("All requests submitted (non-blocking)\n");
    printf("Main thread continues while processes wait for resources...\n\n");

    // Chờ tất cả threads hoàn thành
    for (int i = 0; i < NUM_PROCESSES; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("\n==============================================\n");
    printf("All processes completed\n");
    printf("Final available resources: %d\n", available_resources);
    printf("==============================================\n");

    // Dọn dẹp
    pthread_mutex_destroy(&resource_lock);
    pthread_cond_destroy(&resource_cond);

    return 0;
}