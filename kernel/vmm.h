#ifndef _VMM_H_
#define _VMM_H_

#include "riscv.h"

/* --- utility functions for virtual address mapping --- */
int map_pages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm);
// permission codes.
enum VMPermision {
  PROT_NONE = 0,
  PROT_READ = 1,
  PROT_WRITE = 2,
  PROT_EXEC = 4,
};

// the structure of a memory block in user app. it's used for user_vm_malloc and user_vm_free.
typedef struct HMCB {
  struct HMCB *next;
  int size; // the size of the memory block (in unit of page)
  int busy; // whether the memory block is allocated to user app. 0 for free, 1 for allocated.
  int offset; // the offset of the memory block in the virtual address space of the user app.
} HMCB;

uint64 prot_to_type(int prot, int user);
pte_t *page_walk(pagetable_t pagetable, uint64 va, int alloc);
uint64 lookup_pa(pagetable_t pagetable, uint64 va);

/* --- kernel page table --- */
// pointer to kernel page directory
extern pagetable_t g_kernel_pagetable;

void kern_vm_map(pagetable_t page_dir, uint64 va, uint64 pa, uint64 sz, int perm);

// Initialize the kernel pagetable
void kern_vm_init(void);

/* --- user page table --- */
void *user_va_to_pa(pagetable_t page_dir, void *va);
void user_vm_map(pagetable_t page_dir, uint64 va, uint64 size, uint64 pa, int perm);
void user_vm_unmap(pagetable_t page_dir, uint64 va, uint64 size, int free);
// the page fault handler for user app. 
//Author: Gao Xiaoyang
uint64 user_vm_malloc(pagetable_t page_dir, uint64 pre_size, uint64 now_size);

#endif
