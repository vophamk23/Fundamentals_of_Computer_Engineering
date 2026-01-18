/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

/*
 * System Library
 * Memory Module Library libmem.c 
 */

#include "string.h"
#include "mm.h"
#include "mm64.h"
#include "syscall.h"
#include "libmem.h"
#include "os-cfg.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#ifndef PAGING_PAGE_SWAPPED
#define PAGING_PAGE_SWAPPED(pte) ((pte) & PAGING_PTE_SWAPPED_MASK)
#endif

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/* enlist_vm_freerg_list - add new rg to freerg_list */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = rg_elmt;

  return 0;
}

/* get_symrg_byid - get mem region by region ID */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/* __alloc - allocate a region memory */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, addr_t size, addr_t *alloc_addr)
{
  /* Allocate at the toproof */
  pthread_mutex_lock(&mmvm_lock);
  struct vm_rg_struct rgnode;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
  int inc_sz = 0;

  // 1. Thử tái sử dụng vùng nhớ cũ (Free List)
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->krnl->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->krnl->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
 
    *alloc_addr = rgnode.rg_start;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  // 2. Nếu không có, phải tăng giới hạn bộ nhớ (sbrk)
#ifdef MM64
  inc_sz = (uint32_t)(size / (int)PAGING64_PAGESZ);
  inc_sz = inc_sz + 1; // Tính số trang cần tăng
#else
  inc_sz = PAGING_PAGE_ALIGNSZ(size); // 32-bit tính theo byte aligned
#endif

  int old_sbrk = cur_vma->sbrk;

  /* [UPDATED] GỌI SYSCALL 18 (MEMINC) - Chuẩn Module Hóa */
  struct sc_regs regs;
  regs.a1 = vmaid;
  // regs.a1 = SYSMEM_INC_OP; 
#ifdef MM64
  regs.a2 = size; // 64-bit gửi size thô
#else
  regs.a2 = inc_sz; // 32-bit gửi aligned size (tùy implementation kernel)
  // regs.a2 = vmaid;
#endif  
  regs.a3 = 0;
  // regs.a3 = size;

  // Gọi syscall số 18 để tăng bộ nhớ

  syscall(caller->krnl, caller->pid, 18, &regs); 

  if (regs.a3 < 0) { // Kiểm tra lỗi trả về từ kernel
      pthread_mutex_unlock(&mmvm_lock);
      return -1;
  }

  /* Successful increase limit */
  caller->krnl->mm->symrgtbl[rgid].rg_start = old_sbrk;
  caller->krnl->mm->symrgtbl[rgid].rg_end = old_sbrk + size;

  *alloc_addr = old_sbrk;

  pthread_mutex_unlock(&mmvm_lock);
  return 0;
}

/* __free - remove a region memory */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  pthread_mutex_lock(&mmvm_lock);

  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  struct vm_rg_struct *rgnode = get_symrg_byid(caller->krnl->mm, rgid);

  if (rgnode->rg_start == 0 && rgnode->rg_end == 0)
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  struct vm_rg_struct *freerg_node = malloc(sizeof(struct vm_rg_struct));
  freerg_node->rg_start = rgnode->rg_start;
  freerg_node->rg_end = rgnode->rg_end;
  freerg_node->rg_next = NULL;

  rgnode->rg_start = rgnode->rg_end = 0;
  rgnode->rg_next = NULL;

  /* Enlist the obsoleted memory region */
  enlist_vm_freerg_list(caller->krnl->mm, freerg_node);

  pthread_mutex_unlock(&mmvm_lock);
  return 0;
}

/* liballoc - Wrapper */
int liballoc(struct pcb_t *proc, addr_t size, uint32_t reg_index)
{
  // [UPDATED] Đưa printf lên đầu để dễ debug
  printf("%s:%d\n", __func__, __LINE__); 
  print_pgtbl(proc, 0, -1);
  
  addr_t addr;
  int val = __alloc(proc, 0, reg_index, size, &addr);
  
  if (val == -1)
  {
    
    return -1;
  }

#ifdef IODUMP
#ifdef PAGETBL_DUMP
  //print_pgtbl(proc, 0, -1); 
#endif
#endif

  return val;
}

/* libfree - Wrapper */
int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  // [UPDATED] Đưa printf lên đầu
  printf("%s:%d\n",__func__,__LINE__);
  
  int val = __free(proc, 0, reg_index);
  
  if (val == -1)
  {
    print_pgtbl(proc, 0, -1);
    return -1;
  }

#ifdef IODUMP
#ifdef PAGETBL_DUMP
  //print_pgtbl(proc, 0, -1); 
#endif
#endif
  return 0;
}

/* pg_getpage - Handle Page Fault & Swap */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = pte_get_entry(caller, pgn);

  if (!PAGING_PAGE_PRESENT(pte))
  { 
    /* Trang chưa có trong RAM -> Phải Swap In */
    addr_t vicpgn; // Trang bị đuổi (Victim)
    //addr_t swpfpn; // Vị trí trống trên Swap

    /* 1. Tìm nạn nhân */
    if (find_victim_page(caller->krnl->mm, &vicpgn) == -1)
    {
      return -1;
    }

    /* 2. Tìm chỗ trống trên Swap */
    // Note: Ở đây ta chỉ kiểm tra xem có chỗ không, việc cấp phát thật để Kernel lo
    // Hoặc nếu muốn chắc chắn, ta lấy luôn frame swap ở đây
    // if (MEMPHY_get_freefp(caller->krnl->active_mswp, &swpfpn) == -1) return -1;

    /* 3. [UPDATED] GỌI SYSCALL 19 (MEMSWP) - Bảo mật */
    struct sc_regs regs;
    regs.a1 = vicpgn; // Trang bị đuổi
    regs.a2 = pgn;    // Trang cần nạp (Target)
    regs.a3 = 0;      // Kết quả

    // Gọi syscall hoán đổi trang (Kernel sẽ lo việc tìm swpfpn và copy dữ liệu)
    syscall(caller->krnl, caller->pid, 19, &regs);

    if (regs.a3 < 0) return -1;

    /* 4. Cập nhật Page Table (Logic giả định Kernel đã swap xong) */
    // Lấy swpfpn mà Kernel đã dùng (giả sử Kernel trả về trong a2 hoặc ta tự quản lý)
    // Ở mức đơn giản, ta chỉ cần đảm bảo syscall thành công là được.
    
    // Cập nhật danh sách FIFO
    enlist_pgn_node(&caller->krnl->mm->fifo_pgn, pgn);
  }

  *fpn = PAGING_FPN(pte_get_entry(caller, pgn));

  return 0;
}

/* pg_getval - Read value (Physical) */

int pg_getval(struct mm_struct* mm, int addr, BYTE* data, struct pcb_t* caller)
{
    int pgn = PAGING_PGN(addr);
    int off = PAGING_OFFST(addr);
    int fpn;

    /* Đảm bảo trang có trong RAM */
    if (pg_getpage(mm, pgn, &fpn, caller) != 0)
        return -1;

    // [FIX QUAN TRỌNG] Ép kiểu về unsigned long để tránh tràn số int
    // Đảm bảo fpn hợp lệ
#ifdef MM64
    unsigned long phyaddr = ((unsigned long)fpn * PAGING64_PAGESZ) + off;
#else
    unsigned long phyaddr = ((unsigned long)fpn * PAGING_PAGESZ) + off;
#endif

    /* GỌI SYSCALL 17 (MEMIO READ) */
    struct sc_regs regs;
    regs.a1 = SYSMEM_IO_READ;
    regs.a2 = phyaddr;
    regs.a3 = 0;

    syscall(caller->krnl, caller->pid, 17, &regs);

    *data = (BYTE)regs.a3;

    return 0;
}

/* pg_setval - Write value (Physical) */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Đảm bảo trang có trong RAM */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; 

  // [FIX] Tương tự như trên
#ifdef MM64
  unsigned long phyaddr = ((unsigned long)fpn * PAGING64_PAGESZ) + off;
#else
  unsigned long phyaddr = ((unsigned long)fpn * PAGING_PAGESZ) + off;
#endif

  /* [UPDATED] GỌI SYSCALL 17 (MEMIO WRITE) - Bảo mật */
  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_WRITE;
  regs.a2 = phyaddr;
  regs.a3 = value; // Dữ liệu cần ghi

  syscall(caller->krnl, caller->pid, 17, &regs);

  return 0;
}

/* __read - read value in region memory */
int __read(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->krnl->mm, rgid);

  if (currg == NULL) 
      return -1;

  return pg_getval(caller->krnl->mm, currg->rg_start + offset, data, caller);
}

/* libread - Wrapper */
int libread(struct pcb_t *proc, uint32_t source, addr_t offset, uint32_t* destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  *destination = (uint32_t)data;
  
  printf("%s:%d\n", __func__, __LINE__);
#ifdef IODUMP
#ifdef PAGETBL_DUMP
  //print_pgtbl(proc, 0, -1); 
#endif
#endif

  return val;
}

/* __write - write a region memory */
int __write(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE value)
{
  pthread_mutex_lock(&mmvm_lock);
  struct vm_rg_struct *currg = get_symrg_byid(caller->krnl->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) 
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  int ret = pg_setval(caller->krnl->mm, currg->rg_start + offset, value, caller);

  pthread_mutex_unlock(&mmvm_lock);
  return ret;
}

/* libwrite - Wrapper */
int libwrite(struct pcb_t *proc, BYTE data, uint32_t destination, addr_t offset)
{
  int val = __write(proc, 0, destination, offset, data);
  
  printf("%s:%d\n", __func__, __LINE__);
  if (val == -1)
  {
    print_pgtbl(proc, 0, -1);
    return -1;
  }
#ifdef IODUMP
#ifdef PAGETBL_DUMP
  //print_pgtbl(proc, 0, -1); 
#endif
  MEMPHY_dump(proc->krnl->mram); // Dump RAM để kiểm tra ghi thành công
#endif

  return val;
}

/* free_pcb_memphy - collect all memphy of pcb */
int free_pcb_memph(struct pcb_t *caller)
{
  pthread_mutex_lock(&mmvm_lock);
  int pagenum, fpn;
  uint32_t pte;

  for (pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte = caller->krnl->mm->pgd[pagenum];

    if (PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_FPN(pte);
      MEMPHY_put_freefp(caller->krnl->mram, fpn);
    }
    else if (PAGING_PAGE_SWAPPED(pte)) // Kiểm tra bit swap
    {
      fpn = PAGING_SWP(pte);
      MEMPHY_put_freefp(caller->krnl->active_mswp, fpn);
    }
  }

  pthread_mutex_unlock(&mmvm_lock);
  return 0;
}

/* find_victim_page - FIFO replacement */
int find_victim_page(struct mm_struct *mm, addr_t *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  if (!pg) return -1;
  
  /* Implement FIFO: The last element is the victim */
  struct pgn_t *prev = NULL;
  
  // Special case: List has only 1 item
  if (pg->pg_next == NULL) {
      *retpgn = pg->pgn;
      mm->fifo_pgn = NULL; 
      free(pg);
      return 0;
  }

  // Traverse to find the last node
  while (pg->pg_next)
  {
    prev = pg;
    pg = pg->pg_next;
  }
  
  *retpgn = pg->pgn;
  prev->pg_next = NULL; // Remove last node

  free(pg);

  return 0;
}

/* get_free_vmrg_area - get a free vm region */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL) return -1;

  newrg->rg_start = newrg->rg_end = -1;

  while (rgit != NULL)
  {
    if (rgit->rg_start + size <= rgit->rg_end)
    { 
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;

      if (rgit->rg_start + size < rgit->rg_end)
      {
        rgit->rg_start = rgit->rg_start + size;
      }
      else
      { 
        struct vm_rg_struct *nextrg = rgit->rg_next;
        if (nextrg != NULL)
        {
          rgit->rg_start = nextrg->rg_start;
          rgit->rg_end = nextrg->rg_end;
          rgit->rg_next = nextrg->rg_next;
          free(nextrg);
        }
        else
        {                                
          rgit->rg_start = rgit->rg_end; 
          rgit->rg_next = NULL;
        }
      }
      break;
    }
    else
    {
      rgit = rgit->rg_next; 
    }
  }

  if (newrg->rg_start == -1) return -1;

  return 0;
}