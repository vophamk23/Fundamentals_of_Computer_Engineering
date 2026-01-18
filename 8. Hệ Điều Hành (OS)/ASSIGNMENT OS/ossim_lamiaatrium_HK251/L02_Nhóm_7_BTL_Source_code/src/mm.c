/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

/* NOTICE this moudle is deprecated in LamiaAtrium release
 *        the structure is maintained for future 64bit-32bit
 *        backward compatible feature or PAE feature 
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

#if !defined(MM64)
/*
 * PAGING based Memory Management
 * Memory management unit mm/mm.c
 */

/*
 * init_pte - Initialize PTE entry
 */
int init_pte(addr_t *pte,
             int pre,       // present
             addr_t fpn,    // FPN
             int drt,       // dirty
             int swp,       // swap
             int swptyp,    // swap type
             addr_t swpoff) // swap offset
{
  if (pre != 0)
  {
    if (swp == 0)
    { // Non swap ~ page online
      if (fpn == 0)
        return -1; // Invalid setting

      /* Valid setting with FPN */
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
    }
    else
    { // page swapped
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
      SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
    }
  }

  return 0;
}

/*
 * get_pd_from_pagenum - Parse address to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table
 */
int get_pd_from_address(addr_t addr, addr_t *pgd, addr_t *p4d, addr_t *pud, addr_t *pmd, addr_t *pt)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);

  // need to modify more ----------------------------------------------------------
  // const unsigned PAGE_SHIFT = 12;    /* 4KB pages */
  // const unsigned LEVEL_BITS  = 9;    /* 5-level, 9 bits/level */
  // const unsigned long long LEVEL_MASK = ((1ULL << LEVEL_BITS) - 1ULL);

  // unsigned long long v = (unsigned long long)addr;
  // unsigned long long pgn = v >> PAGE_SHIFT; /* page number */

  // unsigned long long idx_pgd = (pgn >> (LEVEL_BITS * 4)) & LEVEL_MASK;
  // unsigned long long idx_p4d = (pgn >> (LEVEL_BITS * 3)) & LEVEL_MASK;
  // unsigned long long idx_pud = (pgn >> (LEVEL_BITS * 2)) & LEVEL_MASK;
  // unsigned long long idx_pmd = (pgn >> (LEVEL_BITS * 1)) & LEVEL_MASK;
  // unsigned long long idx_pt  = (pgn >> (LEVEL_BITS * 0)) & LEVEL_MASK;

  // if (pgd) *pgd = (addr_t)idx_pgd;
  // if (p4d) *p4d = (addr_t)idx_p4d;
  // if (pud) *pud = (addr_t)idx_pud;
  // if (pmd) *pmd = (addr_t)idx_pmd;
  // if (pt)  *pt  = (addr_t)idx_pt;
  // return 0;

  addr_t pgn = PAGING_PGN(addr);

  /* Với mô hình bảng trang phẳng hiện tại, pgd chính là pgn */
  if (pgd)
    *pgd = pgn;

  /* Các cấp khác không dùng đến trong lab này */
  if (p4d)
    *p4d = 0;
  if (pud)
    *pud = 0;
  if (pmd)
    *pmd = 0;
  if (pt)
    *pt = 0;

  return 0;
}

/*
 * get_pd_from_pagenum - Parse page number to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table
 */
int get_pd_from_pagenum(addr_t pgn, addr_t *pgd, addr_t *p4d, addr_t *pud, addr_t *pmd, addr_t *pt)
{
  /* PGN đã có sẵn, chỉ cần gán */
  if (pgd)
    *pgd = pgn;

  if (p4d)
    *p4d = 0;
  if (pud)
    *pud = 0;
  if (pmd)
    *pmd = 0;
  if (pt)
    *pt = 0;

  return 0;
}

/*
 * pte_set_swap - Set PTE entry for swapped page
 * @pte    : target page table entry (PTE)
 * @swptyp : swap type
 * @swpoff : swap offset
 */
int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff)
{
  struct krnl_t *krnl = caller->krnl;
  addr_t *pte = &krnl->mm->pgd[pgn];

  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
  SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);

  return 0;
}

/*
 * pte_set_swap - Set PTE entry for on-line page
 * @pte   : target page table entry (PTE)
 * @fpn   : frame page number (FPN)
 */
int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn)
{
  struct krnl_t *krnl = caller->krnl;
  addr_t *pte = &krnl->mm->pgd[pgn];

  *pte = 0; //ADD THÊM

  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

  return 0;
}

/* Get PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
uint32_t pte_get_entry(struct pcb_t *caller, addr_t pgn)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct krnl_t *krnl = caller->krnl;
  return krnl->mm->pgd[pgn];
}

/* Set PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
int pte_set_entry(struct pcb_t *caller, addr_t pgn, uint32_t pte_val)
{
  struct krnl_t *krnl = caller->krnl;
  krnl->mm->pgd[pgn] = pte_val;

  return 0;
}

/*
 * vmap_pgd_memset - map a range of page at aligned address
 */
int vmap_pgd_memset(struct pcb_t *caller, // process call
                    addr_t addr,          // start address which is aligned to pagesz
                    int pgnum)            // num of mapping page
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/*
 * vmap_page_range - map a range of page at aligned address
 */
addr_t vmap_page_range(struct pcb_t *caller,           // process call
                       addr_t addr,                    // start address which is aligned to pagesz
                       int pgnum,                      // num of mapping page
                       struct framephy_struct *frames, // list of the mapped frames
                       struct vm_rg_struct *ret_rg)    // return mapped region, the real mapped fp
{                                                      // no guarantee all given pages are mapped
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct framephy_struct *fpit = frames;
  int pgit = 0;
  addr_t pgn = PAGING_PGN(addr);

  ret_rg->rg_start = addr;
  ret_rg->rg_end = addr + pgnum * PAGING_PAGESZ;

  for (pgit = 0; pgit < pgnum; pgit++)
  {
    if (fpit == NULL)
    {
      return -1;
    }
    pte_set_fpn(caller, pgn + pgit, fpit->fpn);
    enlist_pgn_node(&caller->krnl->mm->fifo_pgn, pgn + pgit);//ADD
    fpit = fpit->fp_next;
  }
  return 0;
}

/*
 * alloc_pages_range - allocate req_pgnum of frame in ram
 * @caller    : caller
 * @req_pgnum : request page num
 * @frm_lst   : frame list
 */

addr_t alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  int pgit, fpn;
  struct framephy_struct *newfp_str = NULL; // pointer to a frame node
  printf("[DEBUG] alloc_pages_range: Requesting %d pages for PID %d\n", req_pgnum, caller->pid);  
  // int max_frames = caller->krnl->mram->maxsz / PAGING_PAGESZ;

  for (pgit = 0; pgit < req_pgnum; pgit++)
  {
    newfp_str = malloc(sizeof(struct framephy_struct));
    if (newfp_str == NULL){
      printf("[DEBUG] alloc_pages_range: Malloc failed!\n");
      return -1;
    }
    newfp_str->fp_next = NULL;

    if (MEMPHY_get_freefp(caller->krnl->mram, &fpn) == 0)
    {
      newfp_str->fpn = fpn;
    }
    else
    {
      printf("[DEBUG] alloc_pages_range: RAM FULL. Trying to SWAP...\n");
      int vicpgn;
      int swpfpn;

      if (find_victim_page(caller->krnl->mm, &vicpgn) != 0)
      {
        // No victim page found, allocation fails
        printf("[DEBUG] alloc_pages_range: Failed to find victim (Memory Exhausted)\n");
        free(newfp_str);
        return -1;
      }

      if (MEMPHY_get_freefp(caller->krnl->active_mswp, &swpfpn) != 0)
      {
        // No free swap page found, swap full allocation fails
        printf("[DEBUG] alloc_pages_range: SWAP FULL!\n");
        free(newfp_str);
        return -3000;
      }

      uint32_t vic_pte = caller->krnl->mm->pgd[vicpgn];
      int vicfpn = PAGING_FPN(vic_pte);

      __swap_cp_page(caller->krnl->mram, vicfpn, caller->krnl->active_mswp, swpfpn);

      pte_set_swap(caller, vicpgn, 0, swpfpn);
      newfp_str->fpn = vicfpn;
      printf("[DEBUG] alloc_pages_range: Swapped victim page %d to swap frame %d. Took frame %d.\n", vicpgn, swpfpn, vicfpn);
    }

    newfp_str->owner = caller->krnl->mm;

    if (*frm_lst == NULL)
    {
      *frm_lst = newfp_str;
    }
    else
    {
      // Thêm vào đầu
      // newfp_str->fp_next = *frm_lst;
      // *frm_lst = newfp_str;
      
      // Thêm vào đuôi
      struct framephy_struct *tail = *frm_lst;
      while (tail->fp_next != NULL)
      {
        tail = tail->fp_next;
      }
      // Nối node mới vào sau đuôi
      tail->fp_next = newfp_str;
    }
  }
  printf("[DEBUG] alloc_pages_range: SUCCESS\n");
  return 0;
}

/*
 * vm_map_ram - do the mapping all vm are to ram storage device
 * @caller    : caller
 * @astart    : vm area start
 * @aend      : vm area end
 * @mapstart  : start mapping point
 * @incpgnum  : number of mapped page
 * @ret_rg    : returned region
 */
addr_t vm_map_ram(struct pcb_t *caller, addr_t astart, addr_t aend, addr_t mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct framephy_struct *frames = NULL;
  // addr_t ret_mem = 0; // Địa chỉ trả về (nếu cần)

  /* 1. Kiểm tra tính hợp lệ của incpgnum */
  // int max_num_pages = (aend - astart) / PAGING_PAGESZ;
  // if (incpgnum > max_num_pages)
  // {
  //   return -1; // Yêu cầu vượt quá giới hạn vùng nhớ
  // }

  /* 2. Cấp phát khung trang vật lý */
  if (alloc_pages_range(caller, incpgnum, &frames) != 0)
  {
    return -1; // Out of memory
  }

  /* 3. Ánh xạ vùng nhớ ảo vào danh sách frame vừa xin được */
  vmap_page_range(caller, astart, incpgnum, frames, ret_rg);

  /* 4. Cập nhật thông tin Region trả về (nếu chưa được update trong vmap) */
  if (ret_rg)
  {
    ret_rg->rg_start = mapstart;
    ret_rg->rg_end = mapstart + (incpgnum * PAGING_PAGESZ);
    ret_rg->rg_next = NULL;
  }

  return 0;
}

/* Swap copy content page from source frame to destination frame
 * @mpsrc  : source memphy
 * @srcfpn : source physical page number (FPN)
 * @mpdst  : destination memphy
 * @dstfpn : destination physical page number (FPN)
 **/
int __swap_cp_page(struct memphy_struct *mpsrc, addr_t srcfpn,
                   struct memphy_struct *mpdst, addr_t dstfpn)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  int cellidx;
  addr_t addr_src, addr_dst;
  BYTE data;

  addr_src = srcfpn * PAGING_PAGESZ;
  addr_dst = dstfpn * PAGING_PAGESZ;

  for (cellidx = 0; cellidx < PAGING_PAGESZ; cellidx++)
  {
    MEMPHY_read(mpsrc, addr_src + cellidx, &data);
    MEMPHY_write(mpdst, addr_dst + cellidx, data);
  }
  return 0;
}

/*
 *Initialize a empty Memory Management instance
 * @mm:     self mm
 * @caller: mm owner
 */
int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  // struct vm_area_struct *vma0 = malloc(sizeof(struct vm_area_struct));
  // if (vma0 == NULL)
  // {
  //   // Memory allocation failed
  //   return -1;
  // }
  // mm->pgd = malloc(PAGING_MAX_PGN * sizeof(uint32_t));
  // if (mm->pgd == NULL)
  // {
  //   // Memory allocation failed
  //   free(vma0);
  //   return -1;
  // }
  // for (int i = 0; i < PAGING_MAX_PGN; i++)
  // {
  //   mm->pgd[i] = 0; // Initialize all PTEs to 0
  // }

  // vma0->vm_id = 0;
  // vma0->vm_start = 0;
  // vma0->vm_end = 0;
  // vma0->sbrk = 0;
  // vma0->vm_next = NULL;

  // mm->mmap = vma0;
  // return 0;
    //printf("ALLOC: Init mm\n");
  struct vm_area_struct *vma0 = malloc(sizeof(struct vm_area_struct));
  if (vma0 == NULL) {
    // Memory allocation failed
    return -1;
  }
  vma0->vm_id = 0;
  vma0->vm_start = 0;
  vma0->vm_end = caller->bp;
  vma0->sbrk = 0;
  //printf ("\tvmaid: %d vm_start: %d vm_end: %d sbrk: %d\n", vma0->vm_id,vma0->vm_start,   vma0->vm_end ,vma0->sbrk);
  if(mm->mmap == NULL) mm->mmap = vma0;
  else {
    vma0->vm_next = mm->mmap;
    //mm->mmap = vma0->vm_next;
    mm->mmap = vma0;
  }
  mm->pgd = malloc(PAGING_MAX_PGN * sizeof(uint32_t));
  if (mm->pgd == NULL) {
    // Memory allocation failed
    free(vma0);
    return -1;
  }

  for (int i = 0; i < PAGING_MAX_PGN; i++) {
    mm->pgd[i] = 0;
  }

  
  /* By default the owner comes with at least one vma */
  // struct vm_rg_struct *first_rg = init_vm_rg(vma0->vm_start, vma0->sbrk);
  // if (first_rg == NULL) {
  //   // Memory allocation failed
  //   free(vma0);
  //   free(mm->pgd);
  //   return -1;
  // }
  
  //enlist_vm_rg_node(&vma0->vm_freerg_list, first_rg);

  /* TODO update VMA0 next */
  // vma0->next = ...
  vma0->vm_next = NULL;

  /* Point vma owner backward */
  vma0->vm_mm = mm; 

  /* TODO: update mmap */
  
  return 0;
}

struct vm_rg_struct *init_vm_rg(addr_t rg_start, addr_t rg_end)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));
  if (rgnode == NULL)
  {
    return -1;
  }
  rgnode->rg_start = rg_start;
  rgnode->rg_end = rg_end;
  rgnode->rg_next = NULL;
  return rgnode;
}

int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  if (*rglist == NULL)
  {
    *rglist = rgnode;
    rgnode->rg_next = NULL;
  }
  else
  {
    rgnode->rg_next = *rglist;
    *rglist = rgnode;
  }
  return 0;
}

int enlist_pgn_node(struct pgn_t **plist, addr_t pgn)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct pgn_t *new_node = malloc(sizeof(struct pgn_t));
  if (new_node == NULL)
    return -1;

  new_node->pgn = pgn;
  new_node->pg_next = NULL;

  // Thêm vào đầu danh sách
  if (*plist == NULL)
  {
    *plist = new_node;
  }
  else
  {
    new_node->pg_next = *plist;
    *plist = new_node;
  }

  /* Thêm vào cuối danh sách
  if (*plist == NULL) {
    *plist = new_node;
  } else {
    struct pgn_t *curr = *plist;
    while (curr->pg_next != NULL) curr = curr->pg_next;
    curr->pg_next = new_node;
  }
  */

  return 0;
}

/* Hàm pgread: CPU gọi hàm này -> Hàm này gọi __read (trong libmem.c) */
// int pgread(struct pcb_t *proc, uint32_t source, addr_t offset, uint32_t destination)
// {
//   BYTE data;
//   int val = __read(proc, 0, source, offset, &data);

//   destination = (uint32_t)data;
//   return val;
// }

// /*pgwrite */
// int pgwrite(struct pcb_t *proc, BYTE data, uint32_t destination, addr_t offset)
// {
//   return __write(proc, 0, destination, offset, data);
// }

// /*pgalloc */
// int pgalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
// {
//   addr_t addr;
//   return __alloc(proc, 0, reg_index, size, &addr);
// }

// int pgfree_data(struct pcb_t *proc, uint32_t reg_index)
// {
//   return __free(proc, 0, reg_index);
// }

int print_list_fp(struct framephy_struct *ifp)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct framephy_struct *fp = ifp;

  printf("print_list_fp: ");
  if (fp == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (fp != NULL)
  {
    printf("fp[%d]\n", fp->fpn);
    fp = fp->fp_next;
  }
  printf("\n");
  return 0;
}

int print_list_rg(struct vm_rg_struct *irg)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct vm_rg_struct *rg = irg;

  printf("print_list_rg: ");
  if (rg == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (rg != NULL)
  {
    printf("rg[%ld->%ld]\n", rg->rg_start, rg->rg_end);
    rg = rg->rg_next;
  }
  printf("\n");
  return 0;
}

int print_list_vma(struct vm_area_struct *ivma)
{
  // printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct vm_area_struct *vma = ivma;

  printf("print_list_vma: ");
  if (vma == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (vma != NULL)
  {
    printf("va[%ld->%ld]\n", vma->vm_start, vma->vm_end);
    vma = vma->vm_next;
  }
  printf("\n");
  return 0;
}

/* Trong mm.c - Cập nhật bản in chi tiết */
int print_pgtbl(struct pcb_t *caller, uint32_t start, uint32_t end)
{
  int pgn_start, pgn_end;
  int pgit;

  if (caller == NULL) { printf("NULL caller\n"); return -1; }

  /* Tự động lấy sbrk nếu end = -1 */
  if (end == -1) {
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, 0);
    if (cur_vma != NULL) end = cur_vma->sbrk;
    else end = 0;
  }

  pgn_start = PAGING_PGN(start);
  pgn_end = PAGING_PGN(end);

  printf("print_pgtbl: %d - %d\n", start, end);

  for (pgit = pgn_start; pgit < pgn_end; pgit++)
  {
     uint32_t pte = caller->krnl->mm->pgd[pgit];
     
     /* [QUAN TRỌNG] Kiểm tra bit PRESENT thay vì kiểm tra != 0 */
     if (PAGING_PAGE_PRESENT(pte)) 
     {
       printf("\t%08x: %08x (Page %d -> Frame %d)\n", 
              pgit * PAGING_PAGESZ, pte, pgit, PAGING_FPN(pte));
     }
     /* Uncomment dòng dưới để debug xem có trang nào bị bỏ qua không */
     // else { printf("\t%08x: EMPTY (pte=%08x)\n", pgit * PAGING_PAGESZ, pte); }
  }

  return 0;
}

#endif // ndef MM64
