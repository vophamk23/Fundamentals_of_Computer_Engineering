/* test_mm.c - Unit Test Driver for Virtual Memory Module */
//file này dùng để test các hàm trong mm-vm.c
// 1. Bật chế độ Paging để struct pcb_t có trường 'mm'
#define MM_PAGING 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mm.h"
#include "common.h"

// --- BIẾN ĐIỀU KHIỂN MOCK (Để giả lập lỗi) ---
int mock_vm_map_ram_fail = 0; // 0: Thành công, 1: Thất bại

// --- KHAI BÁO PROTOTYPE (Để tránh warning implicit declaration) ---
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, addr_t size, addr_t alignedsz);
int inc_vma_limit(struct pcb_t *caller, int vmaid, addr_t inc_sz);
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, addr_t vmastart, addr_t vmaend);

// --- STUB FUNCTIONS (Hàm giả) ---

// Stub cho vm_map_ram (Khớp với khai báo trong mm.h)
addr_t vm_map_ram(struct pcb_t *caller, addr_t astart, addr_t aend, 
               addr_t mapstart, int incnumpage, struct vm_rg_struct *ret_rg) {
    // Nếu cờ bật lên, giả vờ map thất bại (trả về -1)
    if (mock_vm_map_ram_fail) {
        printf("    [STUB] vm_map_ram FAILED (Simulated)\n");
        return -1;
    }
    printf("    [STUB] vm_map_ram SUCCESS: Start=%d, End=%d, Pages=%d\n", astart, aend, incnumpage);
    return 0; 
}

// Stub cho __swap_cp_page (Để Linker không báo lỗi undefined reference)
int __swap_cp_page(struct memphy_struct *mpsrc, addr_t srcfpn,
                struct memphy_struct *mpdst, addr_t dstfpn) {
    printf("    [STUB] __swap_cp_page called (Fake Swap)\n");
    return 0;
}

// --- HELPER FUNCTIONS (Hàm hỗ trợ tạo dữ liệu giả) ---

// Tạo Process giả với cấu trúc pcb -> krnl -> mm -> mmap
struct pcb_t *create_mock_proc() {
    struct pcb_t *proc = malloc(sizeof(struct pcb_t));
    
    // 1. Cấp phát Kernel giả
    proc->krnl = malloc(sizeof(struct krnl_t));

    // 2. Cấp phát Memory Management giả bên trong Kernel
    proc->krnl->mm = malloc(sizeof(struct mm_struct));

    // 3. Cấp phát VMA list ban đầu (VMA 0: 0 -> 1000)
    proc->krnl->mm->mmap = malloc(sizeof(struct vm_area_struct));
    proc->krnl->mm->mmap->vm_id = 0;
    proc->krnl->mm->mmap->vm_start = 0;
    proc->krnl->mm->mmap->vm_end = 1000;
    proc->krnl->mm->mmap->sbrk = 1000;
    proc->krnl->mm->mmap->vm_next = NULL; 

    return proc;
}

// Hàm thêm vùng nhớ vào danh sách (Dùng cho test overlap)
void add_vma(struct pcb_t *proc, int id, int start, int end) {
    struct vm_area_struct *new_vma = malloc(sizeof(struct vm_area_struct));
    new_vma->vm_id = id;
    new_vma->vm_start = start;
    new_vma->vm_end = end;
    new_vma->sbrk = end;
    
    // Chèn vào đầu danh sách liên kết
    new_vma->vm_next = proc->krnl->mm->mmap; 
    proc->krnl->mm->mmap = new_vma;
}

// --- TEST CASES ---

/* Test 1: Kiểm tra hàm tạo Node vùng nhớ và cập nhật sbrk */
void test_get_vm_area_node_at_brk() {
    printf("Test 1: get_vm_area_node_at_brk...\n");
    struct pcb_t *proc = create_mock_proc();
    
    // Giả sử sbrk đang ở 1000, muốn lấy thêm 256 bytes (aligned)
    struct vm_rg_struct *rg = get_vm_area_node_at_brk(proc, 0, 200, 256);

    if (rg == NULL) {
        printf("--> FAILED: rg is NULL\n");
        exit(1);
    }

    // Kiểm tra logic
    assert(rg->rg_start == 1000);      
    assert(rg->rg_end == 1256);        
    
    // Kiểm tra sbrk thông qua struct krnl (Phải được cập nhật lên 1256)
    assert(proc->krnl->mm->mmap->sbrk == 1256); 

    printf("--> PASSED\n\n");
    
    // Dọn dẹp
    free(proc->krnl->mm->mmap); free(proc->krnl->mm); free(proc->krnl); free(proc); free(rg);
}

/* Test 2: Kiểm tra tăng giới hạn vùng nhớ cơ bản */
void test_inc_vma_limit() {
    printf("Test 2: inc_vma_limit (Basic)...\n");
    struct pcb_t *proc = create_mock_proc();

    // Tăng 100 bytes (sẽ được làm tròn lên 256)
    int ret = inc_vma_limit(proc, 0, 100);

    assert(ret == 0);                  
    // Kiểm tra vm_end (1000 + 256 = 1256)
    assert(proc->krnl->mm->mmap->vm_end == 1256); 

    printf("--> PASSED\n\n");
    free(proc->krnl->mm->mmap); free(proc->krnl->mm); free(proc->krnl); free(proc);
}

/* Test 3: Kiểm tra mở rộng vùng nhớ ID không tồn tại */
void test_inc_invalid_vma_id() {
    printf("Test 3: inc_vma_limit with INVALID ID...\n");
    struct pcb_t *proc = create_mock_proc();
    
    // Process chỉ có VMA id=0, ta thử tăng VMA id=99
    int ret = inc_vma_limit(proc, 99, 100);

    assert(ret == -1); // Phải trả về lỗi
    printf("--> PASSED\n\n");
    
    free(proc->krnl->mm->mmap); free(proc->krnl->mm); free(proc->krnl); free(proc);
}

/* Test 4: Kiểm tra tính toán Alignment (Làm tròn trang) */
void test_alignment_calculation() {
    printf("Test 4: Alignment Calculation...\n");
    struct pcb_t *proc = create_mock_proc();

    // VMA 0 đang kết thúc ở 1000.
    // Xin thêm 300 bytes.
    // 300 bytes > 256 (1 trang) nhưng < 512 (2 trang).
    // Hệ thống phải cấp tròn 2 trang = 512 bytes.
    // Kết quả mới phải là: 1000 + 512 = 1512.

    int ret = inc_vma_limit(proc, 0, 300);

    assert(ret == 0);
    assert(proc->krnl->mm->mmap->vm_end == 1512); 

    printf("--> PASSED\n\n");
    free(proc->krnl->mm->mmap); free(proc->krnl->mm); free(proc->krnl); free(proc);
}

/* Test 5: Kiểm tra Chồng lấn vùng nhớ (Overlap) */
void test_overlap_detection() {
    printf("Test 5: Overlap Detection...\n");
    struct pcb_t *proc = create_mock_proc(); // Tạo VMA 0: [0, 1000]

    // Tạo thêm VMA 1: [2000, 3000]
    add_vma(proc, 1, 2000, 3000);

    // Bây giờ ta cố tình mở rộng VMA 0 thêm 1500 bytes
    // Dự kiến: [0, 1000] -> [0, 2500].
    // Vùng [2000, 2500] sẽ đè lên VMA 1 (bắt đầu tại 2000) -> Phải BÁO LỖI.
    
    int ret = inc_vma_limit(proc, 0, 1500);

    assert(ret == -1); // Phải thất bại
    
    // Kiểm tra lại xem VMA 0 có bị thay đổi không (phải giữ nguyên là 1000)
    // Lưu ý: add_vma chèn vào đầu nên proc->krnl->mm->mmap trỏ tới VMA 1
    // VMA 0 nằm ở next.
    struct vm_area_struct *vma0 = proc->krnl->mm->mmap->vm_next;
    assert(vma0->vm_id == 0);
    assert(vma0->vm_end == 1000); 

    printf("--> PASSED\n\n");
    
    // Cleanup
    free(proc->krnl->mm->mmap->vm_next); free(proc->krnl->mm->mmap);
    free(proc->krnl->mm); free(proc->krnl); free(proc);
}

/* Test 6: Giả lập lỗi hết RAM (Map Failure) */
void test_map_ram_failure() {
    printf("Test 6: Map RAM Failure (Out of Memory)...\n");
    struct pcb_t *proc = create_mock_proc();

    // Bật chế độ "Hết RAM"
    mock_vm_map_ram_fail = 1;

    int ret = inc_vma_limit(proc, 0, 100);

    assert(ret == -1); // Hàm phải trả về lỗi
    
    // Quan trọng: Nếu map thất bại, vm_end KHÔNG được tăng lên
    assert(proc->krnl->mm->mmap->vm_end == 1000);

    printf("--> PASSED\n\n");
    
    // Reset mock & Cleanup
    mock_vm_map_ram_fail = 0;
    free(proc->krnl->mm->mmap); free(proc->krnl->mm); free(proc->krnl); free(proc);
}

/* Test 7: Tăng bộ nhớ liên tiếp (Sequential Allocation) */
void test_sequential_allocation() {
    printf("Test 7: Sequential Allocation...\n");
    struct pcb_t *proc = create_mock_proc(); // End = 1000

    // Lần 1: Tăng 100 bytes (Align -> 256) => End = 1256
    assert(inc_vma_limit(proc, 0, 100) == 0);
    assert(proc->krnl->mm->mmap->vm_end == 1256);

    // Lần 2: Tăng tiếp 100 bytes (Align -> 256) => End = 1256 + 256 = 1512
    assert(inc_vma_limit(proc, 0, 100) == 0);
    assert(proc->krnl->mm->mmap->vm_end == 1512);

    printf("--> PASSED\n\n");
    free(proc->krnl->mm->mmap); free(proc->krnl->mm); free(proc->krnl); free(proc);
}

// --- MAIN ---
int main() {
    printf("\n========================================\n");
    printf("   UNIT TEST RUNNER: MM-VM MODULE       \n");
    printf("========================================\n");
    
    test_get_vm_area_node_at_brk();
    test_inc_vma_limit();
    test_inc_invalid_vma_id();
    test_alignment_calculation();
    test_overlap_detection();
    test_map_ram_failure();
    test_sequential_allocation();
    
    printf("========================================\n");
    printf("   ALL 7 TESTS PASSED SUCCESSFULLY!     \n");
    printf("========================================\n");
    return 0;
}