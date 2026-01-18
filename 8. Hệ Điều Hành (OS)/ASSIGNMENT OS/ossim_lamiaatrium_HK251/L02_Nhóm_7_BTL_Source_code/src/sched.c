/**
 * sched.c - Multi-Level Queue (MLQ) Scheduler
 *
 * Chính sách MLQ:
 * - 140 mức priority (0=cao nhất, 139=thấp nhất)
 * - Mỗi priority có 1 queue riêng + quota slot
 * - Slot allocation: slot[i] = MAX_PRIO - i
 * - Luôn ưu tiên priority cao trước
 * - Round-robin trong cùng queue
 *
 * Ví dụ: P1(prio=1), P2(prio=0), P3(prio=0)
 *   Queue[0]: [P2,P3] slot=140 (cao nhất, nhiều slot)
 *   Queue[1]: [P1]    slot=139
 *   Thứ tự: P2->P3->P2->P3...->P1 (sau khi queue[0] hết slot)
 */

#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// ========== BIẾN TOÀN CỤC ==========
static struct queue_t ready_queue;
static struct queue_t run_queue;
pthread_mutex_t queue_lock; // Bảo vệ critical section
struct queue_t running_list;

#ifdef MLQ_SCHED
struct queue_t mlq_ready_queue[MAX_PRIO]; // 140 queues cho 140 priorities
static int slot[MAX_PRIO];				  // Quota cho mỗi priority
#endif

/**
 * queue_empty() - Kiểm tra tất cả queues rỗng
 * Return: 0 nếu còn process, -1 nếu hết
 */
int queue_empty(void)
{
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1; // Còn process
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

/**
 * init_scheduler() - Khởi tạo scheduler
 * - Init 140 queues (size=0)
 * - Tính slot: slot[i] = MAX_PRIO - i
 * - Init mutex
 */
void init_scheduler(void)
{
#ifdef MLQ_SCHED
	int i;
	for (i = 0; i < MAX_PRIO; i++)
	{
		mlq_ready_queue[i].size = 0;
		slot[i] = MAX_PRIO - i; // prio=0->slot=140, prio=139->slot=1
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED

// ========== MLQ MODE ==========
/**
 * get_mlq_proc() - Lấy process tiếp theo (CORE ALGORITHM)
 * Return: PCB cần dispatch, NULL nếu không có
 *
 * Thuật toán:
 * 1. Tìm queue có priority cao nhất có process
 * 2. Nếu cao hơn cur_prio -> chuyển ngay (preemption)
 * 3. Nếu còn slot -> lấy từ queue hiện tại
 * 4. Hết slot -> chuyển queue tiếp theo
 *
 * Static variables (GIỮ STATE giữa các lần gọi):
 *   cur_prio: priority đang phục vụ
 *   used_slot: số lần đã lấy từ cur_prio
 *
 * Ví dụ:
 *   Đang phục vụ P1(prio=4), used_slot=50
 *   P2(prio=0) vừa load vào
 *   -> Tìm thấy queue[0] có P2
 *   -> 0 < 4 -> CHUYỂN NGAY cur_prio=0, used_slot=0
 *   -> Dispatch P2 (ưu tiên cao hơn!)
 */
struct pcb_t *get_mlq_proc(void)
{
	struct pcb_t *proc = NULL;

	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	static int used_slot = 0; // Đếm số lần dùng priority hiện tại
	static int cur_prio = 0;  // Priority đang phục vụ

	pthread_mutex_lock(&queue_lock);

	// BƯỚC 1: Tìm queue có priority cao nhất có process
	int highest_prio = -1;
	for (int i = 0; i < MAX_PRIO; i++)
	{
		if (!empty(&mlq_ready_queue[i]))
		{
			highest_prio = i; // Tìm thấy
			break;
		}
	}

	if (highest_prio == -1) // Không có process nào
	{
		pthread_mutex_unlock(&queue_lock);
		return NULL;
	}

	// BƯỚC 2: Priority preemption check
	// Nếu có priority cao hơn -> chuyển ngay
	if (highest_prio < cur_prio)
	{
		cur_prio = highest_prio;
		used_slot = 0; // Reset slot counter
	}

	// BƯỚC 3: Lấy process
	if (!empty(&mlq_ready_queue[cur_prio]) && used_slot < slot[cur_prio])
	{
		// Queue hiện tại còn slot và có process
		proc = dequeue(&mlq_ready_queue[cur_prio]);
		used_slot++;
	}
	else
	{
		// Hết slot hoặc queue rỗng -> tìm queue khác
		used_slot = 0;
		int start_prio = cur_prio;

		// Duyệt vòng tròn tìm queue có process
		do
		{
			cur_prio = (cur_prio + 1) % MAX_PRIO;

			if (!empty(&mlq_ready_queue[cur_prio]))
			{
				proc = dequeue(&mlq_ready_queue[cur_prio]);
				used_slot = 1;
				break;
			}
		} while (cur_prio != start_prio);
	}

	pthread_mutex_unlock(&queue_lock);
	return proc;
}

/**
 * put_mlq_proc() - Trả process về queue
 * Process về queue tương ứng priority: mlq_ready_queue[proc->prio]
 */
void put_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

/**
 * add_mlq_proc() - Thêm process mới vào hệ thống
 */
void add_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

// ========== INTERFACE FUNCTIONS ==========

struct pcb_t *get_proc(void)
{
	return get_mlq_proc();
}

void put_proc(struct pcb_t *proc)
{
	// proc->ready_queue = &ready_queue;
	// proc->mlq_ready_queue = mlq_ready_queue;
	// proc->running_list = &running_list;
	
	proc->krnl->ready_queue = &ready_queue;
	proc->krnl->mlq_ready_queue = mlq_ready_queue;
	proc->krnl->running_list = &running_list;


	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc); // Tracking
	pthread_mutex_unlock(&queue_lock);

	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t *proc)
{
	// proc->ready_queue = &ready_queue;
	// proc->mlq_ready_queue = mlq_ready_queue;
	// proc->running_list = &running_list;

	proc->krnl->ready_queue = &ready_queue;
	proc->krnl->mlq_ready_queue = mlq_ready_queue;
	proc->krnl->running_list = &running_list;


	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc); // Tracking
	pthread_mutex_unlock(&queue_lock);

	return add_mlq_proc(proc);
}

#else

// ========== NON-MLQ MODE (Simple Round-Robin) ==========

struct pcb_t *get_proc(void)
{
	struct pcb_t *proc = NULL;

	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if (!empty(&ready_queue))
	{
		proc = dequeue(&ready_queue);
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_proc(struct pcb_t *proc)
{
	// proc->ready_queue = &ready_queue;
	// proc->running_list = &running_list;

	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc)
{
	// proc->ready_queue = &ready_queue;
	// proc->running_list = &running_list;

	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}
#endif

//////////////////////////////////////////////////////////////