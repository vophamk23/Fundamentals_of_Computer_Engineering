/**
 * test_forkjoin.c - Fork-Join Framework Test
 */
/*
gcc -o test_fj test_forkjoin.c bkfj.c -pthread
./test_fj

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "bkfj.h"

void *square_task(void *arg)
{
    int num = (int)(intptr_t)arg;
    usleep(50000); // Simulate work

    int *result = malloc(sizeof(int));
    *result = num * num;

    printf("  [Worker] %d^2 = %d\n", num, *result);
    return result;
}

void *fib_task(void *arg)
{
    int n = (int)(intptr_t)arg;

    int *result = malloc(sizeof(int));
    if (n <= 1)
    {
        *result = n;
    }
    else
    {
        int a = 0, b = 1;
        for (int i = 2; i <= n; i++)
        {
            int temp = a + b;
            a = b;
            b = temp;
        }
        *result = b;
    }

    printf("  [Worker] fib(%d) = %d\n", n, *result);
    return result;
}

void test_parallel_square()
{
    printf("\n=== TEST 1: Parallel Square Computation ===\n");

    fj_pool_t *pool = fj_pool_init(4);
    printf("Pool initialized with 4 workers\n");

    printf("\nFORK: Submitting 10 tasks...\n");
    clock_t start = clock();

    for (int i = 1; i <= 10; i++)
    {
        fj_fork(pool, square_task, (void *)(intptr_t)i);
    }

    printf("\nJOIN: Waiting for all tasks to complete...\n");
    fj_join(pool);

    clock_t end = clock();
    printf("All tasks completed in %.3f seconds\n",
           ((double)(end - start)) / CLOCKS_PER_SEC);

    fj_pool_destroy(pool);
}

void test_fibonacci()
{
    printf("\n=== TEST 2: Fibonacci Computation ===\n");

    fj_pool_t *pool = fj_pool_init(4);
    printf("Pool initialized with 4 workers\n");

    printf("\nFORK: Computing fib(0) to fib(15)...\n");
    clock_t start = clock();

    for (int i = 0; i <= 15; i++)
    {
        fj_fork(pool, fib_task, (void *)(intptr_t)i);
    }

    printf("\nJOIN: Waiting for all tasks to complete...\n");
    fj_join(pool);

    clock_t end = clock();
    printf("All tasks completed in %.3f seconds\n",
           ((double)(end - start)) / CLOCKS_PER_SEC);

    fj_pool_destroy(pool);
}

void test_multiple_cycles()
{
    printf("\n=== TEST 3: Multiple Fork-Join Cycles ===\n");

    fj_pool_t *pool = fj_pool_init(4);
    printf("Pool initialized with 4 workers\n");

    // Cycle 1
    printf("\n--- Cycle 1 ---\n");
    printf("FORK: Submitting 5 tasks (1^2 to 5^2)\n");
    for (int i = 1; i <= 5; i++)
    {
        fj_fork(pool, square_task, (void *)(intptr_t)i);
    }
    printf("JOIN: Waiting...\n");
    fj_join(pool);
    printf("Cycle 1 completed\n");

    // Cycle 2
    printf("\n--- Cycle 2 ---\n");
    printf("FORK: Submitting 5 tasks (6^2 to 10^2)\n");
    for (int i = 6; i <= 10; i++)
    {
        fj_fork(pool, square_task, (void *)(intptr_t)i);
    }
    printf("JOIN: Waiting...\n");
    fj_join(pool);
    printf("Cycle 2 completed\n");

    fj_pool_destroy(pool);
}

int main()
{
    printf("\n========================================\n");
    printf("  Fork-Join Framework Demonstration\n");
    printf("========================================\n");

    test_parallel_square();
    test_fibonacci();
    test_multiple_cycles();

    printf("\n========================================\n");
    printf("  All tests completed successfully\n");
    printf("========================================\n\n");

    return 0;
}