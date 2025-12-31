/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "pmm.h"
#include "vmm.h"
#include "sched.h"

#include "spike_interface/spike_utils.h"

extern process procs[NPROC];
extern process* ready_queue_head;
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert( current );
  char* pa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)buf);
  sprint(pa);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // reclaim the current process, and reschedule. added @lab3_1
  free_process( current );
  schedule();
  return 0;
}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page() {
  void* pa = alloc_page();
  uint64 va;
  // if there are previously reclaimed pages, use them first (this does not change the
  // size of the heap)
  if (current->user_heap.free_pages_count > 0) {
    va =  current->user_heap.free_pages_address[--current->user_heap.free_pages_count];
    assert(va < current->user_heap.heap_top);
  } else {
    // otherwise, allocate a new page (this increases the size of the heap by one page)
    va = current->user_heap.heap_top;
    current->user_heap.heap_top += PGSIZE;

    current->mapped_info[HEAP_SEGMENT].npages++;
  }
  user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
         prot_to_type(PROT_WRITE | PROT_READ, 1));

  return va;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
  // add the reclaimed page to the free page list
  current->user_heap.free_pages_address[current->user_heap.free_pages_count++] = va;
  return 0;
}

//
// kerenl entry point of naive_fork
//
ssize_t sys_user_fork() {
  sprint("User call fork.\n");
  return do_fork( current );
}

//
// kerenl entry point of yield. added @lab3_2
//
ssize_t sys_user_yield() {
  // TODO (lab3_2): implment the syscall of yield.
  // hint: the functionality of yield is to give up the processor. therefore,
  // we should set the status of currently running process to READY, insert it in
  // the rear of ready queue, and finally, schedule a READY process to run.
  //panic( "You need to implement the yield syscall in lab3_2.\n" );
  
  // set current process to READY and insert it to ready queue
  current->status = READY;
  insert_to_ready_queue( current );
  // schedule another process to run
  schedule();
  return 0;
}

//
// kernel entry of wait.. added @ lab3_challenge1

ssize_t sys_user_wait(int64 pid) {
  //sprint("111\n");
  if(pid > NPROC || pid == 0 || pid < -1) {
    return -1;
  } 
  //sprint("222\n");
  while(1) {

    int flag = 0;
    for (int i=0; i < NPROC; i++) {
      process *p=&procs[i];

      if (p->status == FREE) continue;
      //sprint("333\n");
      if (p->parent == current) {
        if (pid == -1 || p->pid == pid) {
          flag = 1;
          if (p->status == ZOMBIE) {
            int return_pid = p->pid;
            p->status = FREE;
            p->parent = NULL;
            return return_pid;
          }
        }
      }
    }
    if (flag) {
      current->status = BLOCKED;
      schedule();
    } else {
      return -1;
    }
  }
}
/*ssize_t sys_user_wait(int64 pid) {
  //THIS_IS_A_SYNTAX_ERROR!!!!;
  sprint("111\n");
  if(pid > NPROC || pid == 0 || pid < -1) panic("pid is illegal!\n");


  for(process *p = ready_queue_head;;p = p->queue_next) {
    if(p->pid == pid || pid == -1) {
      current->status = BLOCKED;
      schedule();
      break;
    }
    
    if(p->queue_next == NULL) {
      break;
    }
  }
  return 0;
}*/

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    // added @lab2_2
    case SYS_user_allocate_page:
      return sys_user_allocate_page();
    case SYS_user_free_page:
      return sys_user_free_page(a1);
    case SYS_user_fork:
      return sys_user_fork();
    case SYS_user_yield:
      return sys_user_yield();
    case SYS_user_wait:
      return sys_user_wait(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
