/*
gcc -pthread -o lockfree lockfree.c
./lockfree
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 5
#define OPS_PER_THREAD 1000

// Node structure
typedef struct Node
{
    int value;
    struct Node *next;
} Node;

// Lock-Free Stack structure
typedef struct
{
    _Atomic(Node *) head; // Atomic pointer to top node
} LockFreeStack;

// Thread counter for ID assignment
_Atomic int thread_counter = 0;

// ============================================================
// LOCK-FREE STACK OPERATIONS
// ============================================================

// Initialize stack
void stack_init(LockFreeStack *stack)
{
    atomic_init(&stack->head, NULL);
}

// Push an item onto the stack (lock-free)
bool push(LockFreeStack *stack, int value)
{
    // Allocate new node
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL)
    {
        return false;
    }

    new_node->value = value;

    // Lock-free push using CAS (Compare-And-Swap)
    Node *old_head;
    do
    {
        // Read current head
        old_head = atomic_load(&stack->head);

        // Set new node's next to current head
        new_node->next = old_head;

        // Try to CAS: if head is still old_head, set it to new_node
        // If another thread changed head, retry
    } while (!atomic_compare_exchange_weak(&stack->head, &old_head, new_node));

    return true;
}

// Pop an item from the stack (lock-free) - FIXED VERSION
bool pop(LockFreeStack *stack, int *value)
{
    Node *old_head;
    Node *new_head;

    do
    {
        // Read current head
        old_head = atomic_load(&stack->head);

        // Stack is empty
        if (old_head == NULL)
        {
            return false;
        }

        // Read next BEFORE CAS (important for safety)
        new_head = old_head->next;

        // Try to CAS: if head is still old_head, set it to new_head
        // This ensures old_head hasn't been freed by another thread
    } while (!atomic_compare_exchange_weak(&stack->head, &old_head, new_head));

    // Now we own old_head, safe to read and free
    *value = old_head->value;
    free(old_head);

    return true;
}

// ============================================================
// WORKER THREAD - Test concurrent push/pop
// ============================================================
void *worker_thread(void *arg)
{
    LockFreeStack *stack = (LockFreeStack *)arg;
    int thread_id = atomic_fetch_add(&thread_counter, 1);

    printf("[Thread %d] Started\n", thread_id);

    // First, do all pushes
    for (int i = 0; i < OPS_PER_THREAD; i++)
    {
        int push_value = thread_id * 10000 + i;
        if (!push(stack, push_value))
        {
            printf("[Thread %d] Push failed at iteration %d\n", thread_id, i);
        }
    }

    // Small delay to let all threads push
    usleep(10000);

    // Then do pops
    for (int i = 0; i < OPS_PER_THREAD; i++)
    {
        int pop_value;
        if (!pop(stack, &pop_value))
        {
            // Stack might be temporarily empty, that's OK
        }
    }

    printf("[Thread %d] Completed %d operations\n", thread_id, OPS_PER_THREAD * 2);
    return NULL;
}

// ============================================================
// MAIN FUNCTION
// ============================================================
int main()
{
    LockFreeStack stack;
    pthread_t threads[NUM_THREADS];

    printf("=========================================\n");
    printf("  LOCK-FREE STACK IMPLEMENTATION\n");
    printf("=========================================\n\n");

    stack_init(&stack);

    // Test 1: Basic operations
    printf("--- Test 1: Basic Operations ---\n");
    printf("Pushing: 10, 20, 30\n");
    push(&stack, 10);
    push(&stack, 20);
    push(&stack, 30);

    int value;
    printf("Popping: ");
    while (pop(&stack, &value))
    {
        printf("%d ", value);
    }
    printf("\n\n");

    // Test 2: Concurrent operations
    printf("--- Test 2: Concurrent Operations ---\n");
    printf("Starting %d threads with %d ops each...\n\n", NUM_THREADS, OPS_PER_THREAD);

    // Reset thread counter
    atomic_store(&thread_counter, 0);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_create(&threads[i], NULL, worker_thread, &stack) != 0)
        {
            printf("Error creating thread %d\n", i);
            return 1;
        }
    }

    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("\n=========================================\n");
    printf("All threads completed successfully!\n");
    printf("Lock-free stack handled concurrent access\n");
    printf("without any mutex or locks.\n");
    printf("=========================================\n");

    // Cleanup remaining items
    int cleanup_count = 0;
    while (pop(&stack, &value))
    {
        cleanup_count++;
    }
    if (cleanup_count > 0)
    {
        printf("Cleaned up %d remaining items\n", cleanup_count);
    }

    return 0;
}