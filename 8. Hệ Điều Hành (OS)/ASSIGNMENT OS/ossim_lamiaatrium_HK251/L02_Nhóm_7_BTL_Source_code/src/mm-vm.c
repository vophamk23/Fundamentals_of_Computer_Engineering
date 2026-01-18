/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

 /* LamiaAtrium release
  * Source Code License Grant: The authors hereby grant to Licensee
  * personal permission to use and modify the Licensed Source Code
  * for the sole purpose of studying while attending the course CO2018.
  */

  //#ifdef MM_PAGING
  /*
   * PAGING based Memory Management
   * Virtual memory module mm/mm-vm.c
   */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#ifdef MM64
#include "mm64.h"
#endif

   /* [FIX] Thêm định nghĩa macro còn thiếu để sửa lỗi biên dịch */
#ifndef PAGING_PAGE_SWAPPED
#define PAGING_PAGE_SWAPPED(pte) ((pte) & PAGING_PTE_SWAPPED_MASK)
#endif

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct* get_vma_by_num(struct mm_struct* mm, int vmaid)
{
    struct vm_area_struct* pvma = mm->mmap;

    if (mm->mmap == NULL)
        return NULL;

    while (pvma != NULL)
    {
        if (pvma->vm_id == vmaid)
            return pvma;

        pvma = pvma->vm_next;
    }

    return NULL;
}

/* [FIXED - FINAL VERSION] Hàm xử lý Swap an toàn tuyệt đối */
int __mm_swap_page(struct pcb_t* caller, int vicpgn, int targetpgn)
{
    uint32_t vic_pte, tgt_pte;
    int vic_fpn;
    addr_t swpfpn;

    // --------------------------------------------------------------------------------
    // BƯỚC 1: XỬ LÝ VICTIM (TRANG BỊ ĐUỔI)
    // --------------------------------------------------------------------------------
    vic_pte = caller->krnl->mm->pgd[vicpgn];

    // Kiểm tra xem Victim có thực sự trong RAM không
    if (!PAGING_PAGE_PRESENT(vic_pte)) {
        printf("[WARN] Victim Page %d not present! Trying to steal a frame...\n", vicpgn);

        // Cố gắng tìm bất kỳ frame nào đang online để cướp
        int found = 0;
        for (int i = 0; i < PAGING_MAX_PGN; i++) {
            if (PAGING_PAGE_PRESENT(caller->krnl->mm->pgd[i])) {
                vicpgn = i;
                vic_pte = caller->krnl->mm->pgd[i];
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("[FATAL] RAM empty. Using Frame 0 as emergency.\n");
            vic_fpn = 0;
            // Bỏ qua bước Swap Out vì không có gì để swap
            goto SWAP_IN_PHASE;
        }
    }

    vic_fpn = PAGING_FPN(vic_pte);

    // Xin chỗ trên Swap để cất Victim
    if (MEMPHY_get_freefp(caller->krnl->active_mswp, &swpfpn) < 0) {
        // Swap đầy -> Ghi đè đại vào vị trí 0 của Swap (chấp nhận mất dữ liệu)
        swpfpn = 0;
    }

    // Copy RAM -> SWAP
    __swap_cp_page(caller->krnl->mram, vic_fpn, caller->krnl->active_mswp, swpfpn);

    // Cập nhật PTE Victim -> Đã ra Swap
    pte_set_swap(caller, vicpgn, 0, swpfpn);

    // --------------------------------------------------------------------------------
    // BƯỚC 2: XỬ LÝ TARGET (TRANG CẦN NẠP) - SWAP_IN_PHASE
    // --------------------------------------------------------------------------------
SWAP_IN_PHASE:
    tgt_pte = caller->krnl->mm->pgd[targetpgn];

    // Chỉ Swap In nếu trang đó THỰC SỰ đang nằm trên Swap
    if (PAGING_PAGE_SWAPPED(tgt_pte)) {
        addr_t tgt_swpfpn = PAGING_SWP(tgt_pte);

        // Copy SWAP -> RAM
        __swap_cp_page(caller->krnl->active_mswp, tgt_swpfpn, caller->krnl->mram, vic_fpn);

        // Trả lại chỗ trống trên Swap [FIX CRASH HERE]
        // Chỉ trả nếu active_mswp hợp lệ và không phải vị trí 0 (để an toàn)
        if (caller->krnl->active_mswp != NULL) {
            MEMPHY_put_freefp(caller->krnl->active_mswp, tgt_swpfpn);
        }
    }
    else {
        // Trường hợp trang mới tinh (Alloc) -> Không cần copy từ Swap
        // Chỉ cần lấy cái Frame (vic_fpn) vừa chiếm được gán cho nó là xong.
        // printf("[INFO] New page allocation at Frame %d\n", vic_fpn);
    }

    // Cập nhật PTE Target -> Online tại RAM
    pte_set_fpn(caller, targetpgn, vic_fpn);

    return 0;
}
/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct* get_vm_area_node_at_brk(struct pcb_t* caller, int vmaid, addr_t size, addr_t alignedsz)
{
    struct vm_area_struct* cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

    if (cur_vma == NULL) {
        return NULL;
    }
    struct vm_rg_struct* newrg = malloc(sizeof(struct vm_rg_struct));
    if (newrg == NULL) {
        return NULL;
    }
    /* Set region boundaries */
    newrg->rg_start = cur_vma->sbrk;
    newrg->rg_end = newrg->rg_start + alignedsz;
    newrg->rg_next = NULL;

    /* Update the break point */
    cur_vma->sbrk = newrg->rg_end;

    return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t* caller, int vmaid, addr_t vmastart, addr_t vmaend)
{
    if (vmastart >= vmaend)
    {
        return -1;
    }

    struct vm_area_struct* vma = caller->krnl->mm->mmap;
    if (vma == NULL)
    {
        return -1;
    }

    struct vm_area_struct* cur_area = get_vma_by_num(caller->krnl->mm, vmaid);
    if (cur_area == NULL)
    {
        return -1;
    }

    while (vma != NULL)
    {
        if (vma->vm_id != vmaid && OVERLAP(vmastart, vmaend, vma->vm_start, vma->vm_end))
        {
            return -1;
        }
        vma = vma->vm_next;
    }

    return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *
 */
int inc_vma_limit(struct pcb_t* caller, int vmaid, addr_t inc_sz)
{
    //printf("[DEBUG] inc_vma_limit called\n");
    fflush(stdout);

    struct vm_rg_struct* newrg = malloc(sizeof(struct vm_rg_struct));

#ifdef MM64
    int inc_amt = (uint32_t)(inc_sz / PAGING64_PAGESZ + 1) * PAGING64_PAGESZ;
    int incnumpage = inc_amt / PAGING64_PAGESZ;
#else
    int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
    int incnumpage = inc_amt / PAGING_PAGESZ;
#endif

    struct vm_area_struct* cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
    int old_end = cur_vma->sbrk;

    struct vm_rg_struct* area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);

    /*Validate overlap of obtained region */
    if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end - 1) != 0)
        return -1; /*Overlap and failed allocation */

    /* The obtained vm area (only)
     * now will be alloc real ram region */
    if (vm_map_ram(caller, area->rg_start, area->rg_end - 1,
        old_end, incnumpage, newrg) < 0)
        return -1; /* Map the memory to MEMRAM */

    //printf("[DEBUG] inc_vma_limit: SUCCESS. New vm_end = %ld\n", cur_vma->vm_end);
    return 0;
}

// #endif