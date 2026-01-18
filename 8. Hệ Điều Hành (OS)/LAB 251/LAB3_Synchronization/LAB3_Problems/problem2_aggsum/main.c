/*
./aggsum 100 4 1024
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#include "utils.h"
#include <errno.h>
#include <limits.h>

extern int processopts(int argc, char **argv, struct _appconf *conf);
extern int tonum(const char *nptr, int *num);
extern int validate_and_split_argarray(int arraysize, int num_thread, struct _range *thread_idx_range);
extern int generate_array_data(int *buf, int arraysize, int seednum);
extern void help(int xcode);

void *sum_worker(void *arg); // SỬA signature
long validate_sum(int arraysize);

/* Global sum buffer */
long sumbuf = 0;
int *shrdarrbuf;
pthread_mutex_t mtx;

void *sum_worker(void *arg)
{
   struct _range *idx_range = (struct _range *)arg;
   int i;
   long local_sum = 0;

   // printf("In worker from %d to %d\n", idx_range->start, idx_range->end);

   // Tính tổng cục bộ
   for (i = idx_range->start; i <= idx_range->end; i++)
   {
      local_sum += shrdarrbuf[i];
   }

   // Lock chỉ khi cập nhật biến global
   pthread_mutex_lock(&mtx);
   sumbuf += local_sum;
   pthread_mutex_unlock(&mtx);

   return NULL;
}

int main(int argc, char *argv[])
{
   int i, arrsz, tnum, seednum;
   char *buf;
   struct _range *thread_idx_range;
   pthread_t *tid;
   int pid;

   if (argc < 3 || argc > 4)
      help(EXIT_SUCCESS);

#if (DBGSTDERR == 1)
   freopen("/dev/null", "w", stderr);
#endif

   processopts(argc, argv, &appconf);

   fprintf(stdout, "%s runs with %s=%d \t %s=%d \t %s=%d\n", PACKAGE,
           ARG1, appconf.arrsz, ARG2, appconf.tnum, ARG3, appconf.seednum);

   thread_idx_range = malloc(appconf.tnum * sizeof(struct _range));
   if (thread_idx_range == NULL)
   {
      printf("Error! memory for index storage not allocated.\n");
      exit(-1);
   }

   if (validate_and_split_argarray(appconf.arrsz, appconf.tnum, thread_idx_range) < 0)
   {
      printf("Error! array index not splitable. Each partition need at least %d item\n", THRSL_MIN);
      exit(-1);
   }

   shrdarrbuf = malloc(appconf.arrsz * sizeof(int));
   if (shrdarrbuf == NULL)
   {
      printf("Error! memory for array buffer not allocated.\n");
      exit(-1);
   }

   if (generate_array_data(shrdarrbuf, appconf.arrsz, appconf.seednum) < 0)
   {
      printf("Error! array index not splitable.\n");
      exit(-1);
   }

   // THÊM: Khởi tạo mutex
   pthread_mutex_init(&mtx, NULL);

   pid = fork();

   if (pid < 0)
   {
      printf("Error! fork failed.\n");
      exit(-1);
   }

   if (pid == 0)
   {
      printf("sequence sum results %ld\n", validate_sum(appconf.arrsz));
      exit(0);
   }

   tid = malloc(appconf.tnum * sizeof(pthread_t));

   // Truyền địa chỉ của thread_idx_range[i]
   for (i = 0; i < appconf.tnum; i++)
      pthread_create(&tid[i], NULL, sum_worker, (void *)&thread_idx_range[i]);

   for (i = 0; i < appconf.tnum; i++)
      pthread_join(tid[i], NULL);

   fflush(stdout);

   printf("%s gives sum result %ld\n", PACKAGE, sumbuf);

   // THÊM: Hủy mutex
   pthread_mutex_destroy(&mtx);

   // THÊM: Giải phóng bộ nhớ
   free(tid);
   free(thread_idx_range);
   free(shrdarrbuf);

   waitpid(pid, NULL, 0);
   exit(0);
}

long validate_sum(int arraysize)
{
   long validsum = 0;
   int i;

   for (i = 0; i < arraysize; i++)
      validsum += shrdarrbuf[i];

   return validsum;
}

/*
./aggsum 100 4 1024
*/
