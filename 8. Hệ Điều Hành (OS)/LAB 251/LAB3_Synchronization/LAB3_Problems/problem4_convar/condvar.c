/*
 gcc -pthread -o condvar condvar.c
 ./condvar
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define BUFFER_SIZE 10      // Kích thước buffer
#define NUM_WRITERS 5       // Số writer threads
#define NUM_READERS 3       // Số reader threads
#define ITEMS_TO_PRODUCE 20 // Tổng số items cần sản xuất

// ============== DỮ LIỆU CHIA SẺ ==============
int buffer[BUFFER_SIZE]; // Buffer lưu dữ liệu
int count = 0;           // Số items hiện tại trong buffer
int total_produced = 0;  // Tổng đã sản xuất
int total_consumed = 0;  // Tổng đã tiêu thụ

pthread_mutex_t mutex;      // Bảo vệ buffer
pthread_cond_t cond_writer; // CV cho writer (buffer đầy)
pthread_cond_t cond_reader; // CV cho reader (buffer rỗng)

// WRITER: Ghi dữ liệu vào buffer
void *writer(void *arg)
{
    int id = *(int *)arg;

    while (1)
    {
        int item = rand() % 100;
        pthread_mutex_lock(&mutex);

        // Nếu đã đủ → thoát
        if (total_produced >= ITEMS_TO_PRODUCE)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Buffer đầy → chờ reader lấy
        while (count == BUFFER_SIZE)
        {
            printf("Writer %d: Buffer FULL, waiting...\n", id);
            pthread_cond_wait(&cond_writer, &mutex);
        }

        // Kiểm tra lại sau khi thức dậy
        if (total_produced >= ITEMS_TO_PRODUCE)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Ghi vào buffer
        buffer[count] = item;
        printf("Writer %d: Wrote %d at position %d (total: %d)\n",
               id, item, count, total_produced + 1);
        count++;
        total_produced++;

        // Đánh thức tất cả readers
        pthread_cond_broadcast(&cond_reader);
        pthread_mutex_unlock(&mutex);

        usleep(100000); // Ngủ 100ms
    }

    printf("Writer %d: DONE\n", id);
    return NULL;
}

// READER: Đọc dữ liệu từ buffer
void *reader(void *arg)
{
    int id = *(int *)arg;

    while (1)
    {
        pthread_mutex_lock(&mutex);

        // Nếu đã đọc đủ → thoát
        if (total_consumed >= ITEMS_TO_PRODUCE)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Buffer rỗng → chờ writer ghi
        while (count == 0)
        {
            // Đã sản xuất đủ VÀ buffer rỗng → không còn gì để đọc
            if (total_produced >= ITEMS_TO_PRODUCE && count == 0)
            {
                pthread_mutex_unlock(&mutex);
                goto done;
            }

            printf("Reader %d: Buffer EMPTY, waiting...\n", id);
            pthread_cond_wait(&cond_reader, &mutex);
        }

        // Đọc từ buffer (LIFO - stack style)
        count--;
        int item = buffer[count];
        printf("Reader %d: Read %d from position %d (total: %d)\n",
               id, item, count, total_consumed + 1);
        total_consumed++;

        // Đánh thức tất cả writers
        pthread_cond_broadcast(&cond_writer);
        pthread_mutex_unlock(&mutex);

        usleep(150000); // Ngủ 150ms
    }

done:
    printf("Reader %d: DONE\n", id);
    return NULL;
}

int main()
{
    pthread_t writers[NUM_WRITERS];
    pthread_t readers[NUM_READERS];
    int writer_ids[NUM_WRITERS];
    int reader_ids[NUM_READERS];

    // Khởi tạo mutex và condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_writer, NULL);
    pthread_cond_init(&cond_reader, NULL);

    srand(time(NULL));

    // Tạo writer threads
    for (int i = 0; i < NUM_WRITERS; i++)
    {
        writer_ids[i] = i;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }

    // Tạo reader threads
    for (int i = 0; i < NUM_READERS; i++)
    {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }

    // Chờ tất cả writers kết thúc
    for (int i = 0; i < NUM_WRITERS; i++)
        pthread_join(writers[i], NULL);

    // Chờ tất cả readers kết thúc
    for (int i = 0; i < NUM_READERS; i++)
        pthread_join(readers[i], NULL);

    // Dọn dẹp
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_writer);
    pthread_cond_destroy(&cond_reader);

    printf("\n=== ALL DONE ===\n");
    printf("Total produced: %d\n", total_produced);
    printf("Total consumed: %d\n", total_consumed);

    return 0;
}