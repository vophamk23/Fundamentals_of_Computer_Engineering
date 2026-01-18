/* sys_memswp.c - Syscall Handler for Memory Swap */
#include "syscall.h"
#include "common.h"
#include "os-mm.h"
#include "mm.h"
#include <stdlib.h>
#include "queue.h" 

int __sys_memswp(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    printf("__sys_memswp called\n");
    fflush(stdout);
    // 1. Lấy tham số từ thanh ghi (theo quy ước trong đề bài)
    // regs->a1: PID của process sở hữu trang bị đuổi (Victim) hoặc trang cần nạp
    // regs->a2: Số hiệu trang (Page Number)
    // (Lưu ý: Tham số cụ thể phụ thuộc vào cách bạn thống nhất với Thành viên 2 & 3)
    int arg1 = regs->a1; 
    int arg2 = regs->a2;

    struct pcb_t *caller = NULL;
    struct queue_t *running_list = krnl->running_list;

    // 2. Tìm Process đang gọi (Logic duyệt mảng chuẩn)
    if (running_list != NULL) {
        for (int i = 0; i < running_list->size; i++) {
            struct pcb_t *proc = running_list->proc[i];
            if (proc != NULL && proc->pid == pid) {
                caller = proc;
                break;
            }
        }
    }

    if (caller == NULL) return -1;

    // 3. Gọi hàm xử lý chính (nằm trong mm-vm.c hoặc mm.c)
    // Hàm này sẽ thực hiện đổi trang giữa RAM và Swap
    int ret = __mm_swap_page(caller, arg1, arg2);
    
    // 4. Trả kết quả
    regs->a3 = ret;
    return ret;
}