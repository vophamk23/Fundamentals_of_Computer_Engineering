/*
gcc -pthread -o logbuf logbuf.c
./logbuf
*/

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define MAX_LOG_LENGTH 10
#define MAX_BUFFER_SLOT 6
#define MAX_LOOPS 30

char logbuf[MAX_BUFFER_SLOT][MAX_LOG_LENGTH]; // Buffer lưu log
int count;                                    // Số log hiện tại trong buffer

// Đồng bộ hóa
pthread_mutex_t mutex; // Bảo vệ buffer
sem_t empty_slots;     // Đếm slot trống (ban đầu = 6)
sem_t filled_slots;    // Đếm slot đã ghi (ban đầu = 0)

struct _args
{
   unsigned int interval; // Chu kỳ flush (microseconds)
};

// Ghi log vào buffer
void *wrlog(void *data)
{
   char str[MAX_LOG_LENGTH];
   int id = *(int *)data;

   usleep(20);

   sem_wait(&empty_slots);     // Chờ có slot trống
   pthread_mutex_lock(&mutex); // Khóa buffer

   sprintf(str, "%d", id);
   strcpy(logbuf[count], str); // Ghi log
   count++;

   pthread_mutex_unlock(&mutex); // Mở khóa
   sem_post(&filled_slots);      // Báo có thêm 1 log

   return NULL;
}

// Xóa buffer và in ra màn hình
void flushlog()
{
   int i;
   char nullval[MAX_LOG_LENGTH];
   int items_to_flush;

   pthread_mutex_lock(&mutex); // Khóa buffer

   items_to_flush = count;
   sprintf(nullval, "%d", -1);

   // In và xóa tất cả log
   for (i = 0; i < items_to_flush; i++)
   {
      printf("Slot  %i: %s\n", i, logbuf[i]);
      strcpy(logbuf[i], nullval);
   }
   fflush(stdout);

   count = 0; // Reset buffer

   pthread_mutex_unlock(&mutex); // Mở khóa

   // Cập nhật semaphore
   for (i = 0; i < items_to_flush; i++)
   {
      sem_wait(&filled_slots); // Giảm số log
      sem_post(&empty_slots);  // Tăng số slot trống
   }

   return;
}

// Thread định kỳ flush buffer
void *timer_start(void *args)
{
   while (1)
   {
      usleep(((struct _args *)args)->interval); // Chờ theo chu kỳ

      int sem_value;
      sem_getvalue(&filled_slots, &sem_value);

      if (sem_value > 0) // Nếu có log → flush
      {
         flushlog();
      }
   }
   return NULL;
}

int main()
{
   int i;
   count = 0;
   pthread_t tid[MAX_LOOPS];
   pthread_t lgrid;
   int id[MAX_LOOPS];

   // Khởi tạo đồng bộ
   pthread_mutex_init(&mutex, NULL);
   sem_init(&empty_slots, 0, MAX_BUFFER_SLOT); // 6 slot trống
   sem_init(&filled_slots, 0, 0);              // 0 slot đã ghi

   struct _args args;
   args.interval = 500e3; // Flush mỗi 500ms

   // Tạo timer thread (flush định kỳ)
   pthread_create(&lgrid, NULL, &timer_start, (void *)&args);

   // Tạo 30 thread ghi log
   for (i = 0; i < MAX_LOOPS; i++)
   {
      id[i] = i;
      pthread_create(&tid[i], NULL, wrlog, (void *)&id[i]);
   }

   // Chờ tất cả thread ghi xong
   for (i = 0; i < MAX_LOOPS; i++)
      pthread_join(tid[i], NULL);

   sleep(5); // Đợi flush lần cuối

   // Dọn dẹp
   pthread_cancel(lgrid);
   pthread_join(lgrid, NULL);
   pthread_mutex_destroy(&mutex);
   sem_destroy(&empty_slots);
   sem_destroy(&filled_slots);

   return 0;
}