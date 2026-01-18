/**
 * queue.c - Hàng đợi FIFO cho MLQ Scheduler
 * Nguyên tắc: First In First Out (vào trước ra trước)
 */

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/**
 * empty() - Kiểm tra queue có rỗng không
 * @q: Queue cần kiểm tra
 * Return: 1 nếu rỗng, 0 nếu có phần tử
 */
int empty(struct queue_t *q)
{
        // Bước 1: Kiểm tra queue hợp lệ
        if (q == NULL)
                return 1;

        // Bước 2: Kiểm tra size
        return (q->size == 0);
}

/**
 * enqueue() - Thêm process vào cuối queue (FIFO)
 * @q: Queue cần thêm
 * @proc: Process cần thêm
 *
 * Ví dụ: [P1, P2] + enqueue(P3) = [P1, P2, P3]
 */
void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        // Bước 1: Kiểm tra tham số hợp lệ
        if (q == NULL || proc == NULL)
                return;

        // Bước 2: Kiểm tra queue đã đầy chưa
        if (q->size >= MAX_QUEUE_SIZE)
        {
                printf("ERROR: Queue is full! Cannot enqueue process PID=%d\n",
                       proc->pid);
                return;
        }

        // Bước 3: Thêm process vào cuối mảng
        // VD: size=2 → thêm vào index 2
        q->proc[q->size] = proc;

        // Bước 4: Tăng size lên 1
        q->size++;
}

/**
 * dequeue() - Lấy process ở đầu queue (FIFO)
 * @q: Queue cần lấy
 * Return: Process đầu tiên, NULL nếu rỗng
 *
 * Ví dụ: [P1, P2, P3] → dequeue() → P1, còn [P2, P3]
 * Độ phức tạp: O(n) do phải dịch chuyển mảng
 */
struct pcb_t *dequeue(struct queue_t *q)
{
        // Bước 1: Kiểm tra queue hợp lệ và không rỗng
        if (q == NULL || q->size == 0)
                return NULL;

        // Bước 2: Lưu lại process đầu tiên (index 0)
        struct pcb_t *proc = q->proc[0];

        // Bước 3: Dịch tất cả phần tử sang trái
        // [P1, P2, P3] → [P2, P3, P3]
        // index 1→0, index 2→1
        for (int i = 1; i < q->size; i++)
        {
                q->proc[i - 1] = q->proc[i];
        }

        // Bước 4: Xóa phần tử cuối (tránh pointer lơ lửng)
        q->proc[q->size - 1] = NULL;

        // Bước 5: Giảm size xuống 1
        q->size--;

        // Bước 6: Trả về process đã lấy
        return proc;
}

/**
 * purgequeue() - Xóa một process CỤ THỂ khỏi queue
 * @q: Queue chứa process
 * @proc: Process cần xóa (so sánh theo PID)
 * Return: Process đã xóa, NULL nếu không tìm thấy
 *
 * Use case: Process bị terminate/block giữa chừng
 * Ví dụ: [P1, P2, P3, P4] + purge(P2) = [P1, P3, P4]
 */
struct pcb_t *purgequeue(struct queue_t *q, struct pcb_t *proc)
{
        // Bước 1: Kiểm tra tham số hợp lệ
        if (q == NULL || proc == NULL || q->size == 0)
                return NULL;

        // Bước 2: Tìm vị trí của process trong queue
        // So sánh theo PID để tìm đúng process
        int found = -1;
        for (int i = 0; i < q->size; i++)
        {
                if (q->proc[i] != NULL && q->proc[i]->pid == proc->pid)
                {
                        found = i; // Lưu vị trí tìm thấy
                        break;
                }
        }

        // Bước 3: Nếu không tìm thấy, return NULL
        if (found == -1)
                return NULL;

        // Bước 4: Lưu lại process cần xóa
        struct pcb_t *removed = q->proc[found];

        // Bước 5: Dịch các phần tử SAU vị trí found sang trái
        // VD: [P1, P2, P3, P4], xóa P2 (found=1)
        // → [P1, P3, P4, P4] (copy P3→1, P4→2)
        for (int i = found; i < q->size - 1; i++)
        {
                q->proc[i] = q->proc[i + 1];
        }

        // Bước 6: Xóa phần tử cuối
        q->proc[q->size - 1] = NULL;

        // Bước 7: Giảm size
        q->size--;

        // Bước 8: Trả về process đã xóa
        return removed;
}

/**
 * print_queue() - In nội dung queue (dùng để debug)
 * @q: Queue cần in
 */
void print_queue(struct queue_t *q)
{
        // Bước 1: Kiểm tra queue hợp lệ
        if (q == NULL)
        {
                printf("Queue: NULL\n");
                return;
        }

        // Bước 2: In thông tin queue
        printf("Queue size: %d\n", q->size);

        // Bước 3: Duyệt và in từng process
        for (int i = 0; i < q->size; i++)
        {
                if (q->proc[i] != NULL)
                {
                        printf("  [%d] PID: %d, Priority: %d\n",
                               i, q->proc[i]->pid, q->proc[i]->prio);
                }
        }
}
