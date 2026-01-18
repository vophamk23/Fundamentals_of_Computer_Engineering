#include "syscall.h"
#include "common.h"
#include "os-mm.h"
#include "mm.h"
#include <stdlib.h>
#include "queue.h" // Vẫn cần include file này

int __sys_meminc(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    //printf("__sys_meminc called\n");
    fflush(stdout);
    int vmaid = regs->a1;
    addr_t inc_sz = regs->a2;
    
    struct pcb_t *caller = NULL;
    if (krnl->running_list == NULL) {
        //printf("[DEBUG] __sys_meminc: No running list in kernel\n");
    }

    struct queue_t *running_list = krnl->running_list;
    
    // Tìm kiếm PCB của tiến trình gọi dựa trên PID
    if (running_list != NULL) {
        // Duyệt qua mảng proc[] dựa trên kích thước size hiện tại
        for (int i = 0; i < running_list->size; i++) {
            struct pcb_t *proc = running_list->proc[i];
            //printf("[DEBUG] __sys_meminc: Checking PID %d ", proc->pid);
            
            // Kiểm tra nếu tìm thấy PID trùng khớp
            if (proc != NULL && proc->pid == pid) {
                caller = proc;
                //printf("[DEBUG] __sys_meminc: Found caller PCB for PID %d\n", pid);
                break; // Tìm thấy rồi thì thoát vòng lặp
            }
        }
        printf("\n");
    }
    // --------------------------------------
    
    if (krnl->ready_queue == NULL) {
        //printf("[DEBUG] __sys_meminc: No ready queue in kernel\n");
    }

    /* Tìm trên ready_queue */
    if (caller == NULL && krnl->ready_queue != NULL)
    {
        for (int i = 0; i < krnl->ready_queue->size; i++)
        {
            struct pcb_t *proc = krnl->ready_queue->proc[i];
            if (proc != NULL && proc->pid == pid)
            {
                caller = proc;
                break;
            }
        }
    }

    if (caller == NULL) {
        //printf("[DEBUG] __sys_meminc: Caller PCB not found for PID %d\n", pid);
        return -1;
    }
    
    int ret = inc_vma_limit(caller, vmaid, inc_sz);
    regs->a3 = ret;
    return ret;
}