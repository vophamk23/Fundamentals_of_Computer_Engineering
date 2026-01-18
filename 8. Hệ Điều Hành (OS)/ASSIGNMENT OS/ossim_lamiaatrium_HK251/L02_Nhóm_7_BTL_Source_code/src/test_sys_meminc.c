/* test_sys_meminc.c */
#define MM_PAGING

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "common.h"
#include "syscall.h"
#include "queue.h"

// --- 1. BIẾN TOÀN CỤC ĐỂ KIỂM TRA KẾT QUẢ (Mocking State) ---
int mock_inc_vma_limit_called = 0; // Biến cờ: =1 nếu hàm được gọi
int last_vmaid = -1;               // Lưu vmaid nhận được
int last_inc_sz = -1;              // Lưu size nhận được

// --- 2. STUB (Hàm giả) ---

// Chúng ta giả lập hàm inc_vma_limit.
// Thay vì tăng bộ nhớ thật, nó chỉ ghi lại thông tin "Tôi đã được gọi".
int inc_vma_limit(struct pcb_t *caller, int vmaid, addr_t inc_sz) {
    printf("    [STUB] inc_vma_limit called for PID=%d, VMAID=%d, Size=%d\n", 
           caller->pid, vmaid, inc_sz);
    
    mock_inc_vma_limit_called = 1; // Đánh dấu là đã gọi
    last_vmaid = vmaid;            // Lưu tham số để kiểm tra
    last_inc_sz = inc_sz;
    
    return 0; // Giả vờ thành công
}

// Khai báo hàm cần test (nằm trong sys_meminc.c)
int __sys_meminc(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs);


// --- 3. HÀM HỖ TRỢ TẠO MÔI TRƯỜNG GIẢ ---
struct krnl_t *create_mock_kernel() {
    struct krnl_t *krnl = malloc(sizeof(struct krnl_t));
    
    // Cấp phát Running List (Queue)
    krnl->running_list = malloc(sizeof(struct queue_t));
    krnl->running_list->size = 0; // Ban đầu hàng đợi rỗng
    
    // Xóa sạch mảng proc để tránh rác
    for(int i=0; i<MAX_QUEUE_SIZE; i++) {
        krnl->running_list->proc[i] = NULL;
    }
    
    return krnl;
}

// Hàm thêm process vào hàng đợi giả
void add_proc_to_queue(struct krnl_t *krnl, int pid) {
    if (krnl->running_list->size >= MAX_QUEUE_SIZE) return;
    
    struct pcb_t *proc = malloc(sizeof(struct pcb_t));
    proc->pid = pid;
    
    // Thêm vào mảng (theo đúng logic queue.h)
    int idx = krnl->running_list->size;
    krnl->running_list->proc[idx] = proc;
    krnl->running_list->size++;
}

// --- 4. CÁC TEST CASE ---

/* Test 1: Tìm thấy Process và gọi hàm thành công */
void test_success_case() {
    printf("Test 1: Found PID and Call Success...\n");
    
    // Setup môi trường
    struct krnl_t *krnl = create_mock_kernel();
    add_proc_to_queue(krnl, 101); // Process khác
    add_proc_to_queue(krnl, 999); // Target Process (Mục tiêu)
    add_proc_to_queue(krnl, 102); // Process khác
    
    // Setup tham số (Giả lập thanh ghi CPU)
    struct sc_regs regs;
    regs.a1 = 1;   // vmaid
    regs.a2 = 500; // inc_sz
    
    // Reset biến cờ
    mock_inc_vma_limit_called = 0;

    // GỌI HÀM CẦN TEST (Tìm PID 999)
    int ret = __sys_meminc(krnl, 999, &regs);

    // KIỂM TRA
    assert(ret == 0);                       // Hàm phải trả về 0
    assert(mock_inc_vma_limit_called == 1); // inc_vma_limit PHẢI được gọi
    assert(last_vmaid == 1);                // vmaid truyền vào phải đúng
    assert(last_inc_sz == 500);             // size truyền vào phải đúng
    assert(regs.a3 == 0);                   // Kết quả trả về trong thanh ghi a3 phải là 0

    printf("--> PASSED\n\n");
    // (Bỏ qua free để code ngắn gọn)
}

/* Test 2: Không tìm thấy Process (PID sai) */
void test_pid_not_found() {
    printf("Test 2: PID Not Found...\n");
    
    struct krnl_t *krnl = create_mock_kernel();
    add_proc_to_queue(krnl, 100);
    
    struct sc_regs regs;
    regs.a1 = 0;
    regs.a2 = 100;

    mock_inc_vma_limit_called = 0;

    // GỌI HÀM (Tìm PID 999 không tồn tại)
    int ret = __sys_meminc(krnl, 999, &regs);

    // KIỂM TRA
    assert(ret == -1);                      // Phải trả về lỗi
    assert(mock_inc_vma_limit_called == 0); // inc_vma_limit KHÔNG ĐƯỢC gọi

    printf("--> PASSED\n\n");
}

/* Test 3: Hàng đợi rỗng (Running List NULL) */
void test_empty_queue() {
    printf("Test 3: Empty Queue...\n");
    
    struct krnl_t *krnl = malloc(sizeof(struct krnl_t));
    krnl->running_list = NULL; // Hàng đợi chưa khởi tạo
    
    struct sc_regs regs;

    int ret = __sys_meminc(krnl, 1, &regs);

    assert(ret == -1);
    printf("--> PASSED\n\n");
}

int main() {
    printf("=== START TEST SYSMEMINC ===\n");
    test_success_case();
    test_pid_not_found();
    test_empty_queue();
    printf("=== ALL TESTS PASSED ===\n");
    return 0;
}