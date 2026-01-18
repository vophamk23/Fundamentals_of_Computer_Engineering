/*
gcc -pthread -o test_multithread test_multithread.c
./test_multithread
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "seqlock.h"

pthread_seqlock_t lock;
int shared_data = 0;

// Writer thread
void *writer_func(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < 5; i++)
    {
        pthread_seqlock_wrlock(&lock);
        printf("[Writer %d] Writing... old=%d\n", id, shared_data);
        shared_data++;
        sleep(1); // Giả lập ghi chậm
        printf("[Writer %d] Done. new=%d\n", id, shared_data);
        pthread_seqlock_wrunlock(&lock);

        sleep(2);
    }

    return NULL;
}

// Reader thread
void *reader_func(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < 10; i++)
    {
        if (pthread_seqlock_rdlock(&lock) == 1)
        {
            int data = shared_data;

            if (pthread_seqlock_rdunlock(&lock) == 1)
            {
                printf("  [Reader %d] Read data = %d (valid)\n", id, data);
            }
            else
            {
                printf("  [Reader %d] Data invalid (writer interrupted)\n", id);
            }
        }
        else
        {
            printf("  [Reader %d] Writer is writing, skipped\n", id);
        }

        sleep(1);
    }

    return NULL;
}

int main()
{
    pthread_t writer, reader1, reader2;
    int w_id = 1, r1_id = 1, r2_id = 2;

    printf("=== Test: 1 Writer + 2 Readers ===\n\n");

    pthread_seqlock_init(&lock);

    // Tạo threads
    pthread_create(&writer, NULL, writer_func, &w_id);
    pthread_create(&reader1, NULL, reader_func, &r1_id);
    pthread_create(&reader2, NULL, reader_func, &r2_id);

    // Chờ threads
    pthread_join(writer, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    printf("\n=== Final shared_data = %d ===\n", shared_data);

    return 0;
}
