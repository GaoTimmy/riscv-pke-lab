// /*
//  * contains the implementation of all syscalls.
//  */

// #include <stdint.h>
// #include <errno.h>

// #include "util/types.h"
// #include "syscall.h"
// #include "string.h"
// #include "process.h"
// #include "util/functions.h"
// #include "pmm.h"
// #include "vmm.h"
// #include "vfs.h"
// #include "sched.h"
// #include "proc_file.h"

// #include "spike_interface/spike_utils.h"

// //
// // implement the SYS_user_print syscall
// //
// ssize_t sys_user_print(const char *buf, size_t n)
// {
//   // buf is now an address in user space of the given app's user stack,
//   // so we have to transfer it into phisical address (kernel is running in direct mapping).
//   assert(current);
//   char *pa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void *)buf);
//   sprint(pa);
//   return 0;
// }

// //
// // implement the SYS_user_exit syscall
// //
// ssize_t sys_user_exit(uint64 code)
// {
//   sprint("User exit with code:%d.\n", code);
//   // reclaim the current process, and reschedule. added @lab3_1
//   free_process(current);
//   schedule();
//   return 0;
// }

// //
// // maybe, the simplest implementation of malloc in the world ... added @lab2_2
// //
// uint64 sys_user_allocate_page()
// {
//   void *pa = alloc_page();
//   uint64 va;
//   // if there are previously reclaimed pages, use them first (this does not change the
//   // size of the heap)
//   if (current->user_heap.free_pages_count > 0)
//   {
//     va = current->user_heap.free_pages_address[--current->user_heap.free_pages_count];
//     assert(va < current->user_heap.heap_top);
//   }
//   else
//   {
//     // otherwise, allocate a new page (this increases the size of the heap by one page)
//     va = current->user_heap.heap_top;
//     current->user_heap.heap_top += PGSIZE;

//     current->mapped_info[HEAP_SEGMENT].npages++;
//   }
//   user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
//               prot_to_type(PROT_WRITE | PROT_READ, 1));

//   return va;
// }

// //
// // reclaim a page, indicated by "va". added @lab2_2
// //
// uint64 sys_user_free_page(uint64 va)
// {
//   user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
//   // add the reclaimed page to the free page list
//   current->user_heap.free_pages_address[current->user_heap.free_pages_count++] = va;
//   return 0;
// }

// //
// // kerenl entry point of naive_fork
// //
// ssize_t sys_user_fork()
// {
//   sprint("User call fork.\n");
//   return do_fork(current);
// }

// //
// // kerenl entry point of yield. added @lab3_2
// //
// ssize_t sys_user_yield()
// {
//   // TODO (lab3_2): implment the syscall of yield.
//   // hint: the functionality of yield is to give up the processor. therefore,
//   // we should set the status of currently running process to READY, insert it in
//   // the rear of ready queue, and finally, schedule a READY process to run.
//   // panic( "You need to implement the yield syscall in lab3_2.\n" );

//   // sprint("User call yield");
//   current->status = READY;
//   insert_to_ready_queue(current);
//   schedule();
//   return 0;
// }

// //
// // open file
// //
// ssize_t sys_user_open(char *pathva, int flags) {
//   char* pathpa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), pathva);
//   return do_open(pathpa, flags);
// }

// //
// // read file
// //
// ssize_t sys_user_read(int fd, char *bufva, uint64 count) {
//   int i = 0;
//   while (i < count) { // count can be greater than page size
//     uint64 addr = (uint64)bufva + i;
//     uint64 pa = lookup_pa((pagetable_t)current->pagetable, addr);
//     uint64 off = addr - ROUNDDOWN(addr, PGSIZE);
//     uint64 len = count - i < PGSIZE - off ? count - i : PGSIZE - off;
//     uint64 r = do_read(fd, (char *)pa + off, len);
//     i += r; if (r < len) return i;
//   }
//   return count;
// }

// //
// // write file
// //
// ssize_t sys_user_write(int fd, char *bufva, uint64 count) {
//   int i = 0;
//   while (i < count) { // count can be greater than page size
//     uint64 addr = (uint64)bufva + i;
//     uint64 pa = lookup_pa((pagetable_t)current->pagetable, addr);
//     uint64 off = addr - ROUNDDOWN(addr, PGSIZE);
//     uint64 len = count - i < PGSIZE - off ? count - i : PGSIZE - off;
//     uint64 r = do_write(fd, (char *)pa + off, len);
//     i += r; if (r < len) return i;
//   }
//   return count;
// }

// //
// // lseek file
// //
// ssize_t sys_user_lseek(int fd, int offset, int whence) {
//   return do_lseek(fd, offset, whence);
// }

// //
// // read vinode
// //
// ssize_t sys_user_stat(int fd, struct istat *istat) {
//   struct istat * pistat = (struct istat *)user_va_to_pa((pagetable_t)(current->pagetable), istat);
//   return do_stat(fd, pistat);
// }

// //
// // read disk inode
// //
// ssize_t sys_user_disk_stat(int fd, struct istat *istat) {
//   struct istat * pistat = (struct istat *)user_va_to_pa((pagetable_t)(current->pagetable), istat);
//   return do_disk_stat(fd, pistat);
// }

// //
// // close file
// //
// ssize_t sys_user_close(int fd) {
//   return do_close(fd);
// }

// //
// // lib call to opendir
// //
// ssize_t sys_user_opendir(char * pathva){
//   char * pathpa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), pathva);
//   rp_to_ap(pathpa, pathpa);
//   return do_opendir(pathpa);
// }

// //
// // lib call to readdir
// //
// ssize_t sys_user_readdir(int fd, struct dir *vdir){
//   struct dir * pdir = (struct dir *)user_va_to_pa((pagetable_t)(current->pagetable), vdir);
//   return do_readdir(fd, pdir);
// }

// //
// // lib call to mkdir
// //
// ssize_t sys_user_mkdir(char * pathva){
//   char * pathpa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), pathva);
//   return do_mkdir(pathpa);
// }

// //
// // lib call to closedir
// //
// ssize_t sys_user_closedir(int fd){
//   return do_closedir(fd);
// }

// //
// // lib call to link
// //
// ssize_t sys_user_link(char * vfn1, char * vfn2){
//   char * pfn1 = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)vfn1);
//   char * pfn2 = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)vfn2);
//   return do_link(pfn1, pfn2);
// }

// //
// // lib call to unlink
// //
// ssize_t sys_user_unlink(char * vfn){
//   char * pfn = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)vfn);
//   return do_unlink(pfn);
// }

// //
// // lib call to ccwd
// //
// ssize_t sys_user_rcwd(uint64 path) {
//   char *pa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void*)path); // 获取path的物理地址
//   // uint64 pa = lookup_pa((pagetable_t)(current->pagetable), (void*)path); 
//   memcpy((char*)pa, current->pfiles->cwd->name, strlen(current->pfiles->cwd->name)); // 将当前工作目录的路径复制到path
//   return 0;
// }

// // 
// // lib call to ccwd
// //
// ssize_t sys_user_ccwd(uint64 path) { // path代表路径,可能是相对路径，也可能是绝对路径
//   char *pa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void*)path); // 获取path的物理地址
//   // uint64 pa = lookup_pa((pagetable_t)(current->pagetable), path); // 获取path的物理地址
//   char absolute_path[MAX_FILE_NAME_LEN + 5]; // 绝对路径，最终存放的必然是绝对路径
//   memset(absolute_path, 0, MAX_FILE_NAME_LEN + 5); // 初始化绝对路径
//   rp_to_ap(pa, absolute_path); // 将相对路径转换为绝对路径
//   memcpy(current->pfiles->cwd->name, absolute_path, strlen(absolute_path)); // 将绝对路径复制到当前工作目录的路径
//   return 0;
// }
// void rp_to_ap(char *relative_path, char *absolute_path) {
//   if(relative_path == NULL || absolute_path == NULL) return;
//   char *ap = absolute_path; // 绝对路径的指针
//   char * cwd = current->pfiles->cwd->name; // 当前工作目录的路径

//   if(strlen(relative_path) == 0) { // 如果rp是空路径
//     strcpy(absolute_path, cwd); // 直接复制
//     return ;
//   }

//   if (relative_path[0] == '/') { // 如果是根目录
//     strcpy(absolute_path, relative_path); // 直接复制
//     return ;
//   } 

//   strcpy(absolute_path, cwd); // 先复制当前工作目录的路径
//   ap += strlen(cwd); // 移动指针

//   if(ap[-1] != '/') { // 如果当前工作目录的路径最后不是/
//     *ap = '/'; // 添加/
//     ap++; // 移动指针
//   }
//   char *rp = relative_path; 
//   while(*rp != 0) {
//     if(*rp == '.' && *(rp + 1) == '.' && *(rp + 2) == '/') { // 如果是../
//       if(ap == absolute_path) { // 如果是根目录
//         rp += 3; // 移动指针
//         continue;
//       }
//       ap--; // 移动指针
//       while (ap > absolute_path && *ap != '/') ap --;
//       rp += 3; // 移动指针
//     }
//       else if(*rp == '.' && *(rp + 1) == '/') { // 如果是./
//         rp += 2; // 移动指针
//       } 
//       else if(*rp == '/'){
//         rp++; // 跳过/
//       }
//       else { // 如果是普通路径
//         // 复制路径段到绝对路径
//         while (*rp != '\0' && *rp != '/') {
//             *ap = *rp;
//             ap++;
//             rp++;
//         }
//         if (*rp == '/') { // 如果路径段后有 '/'，添加 '/'
//             *ap = '/';
//             ap++;
//             rp++;
//         }
//       }
//   }

//   if(ap > absolute_path && ap[-1] == '/') { // 如果绝对路径最后有'/'
//     ap--; // 移动指针
//   }
//   *ap = '\0'; // 添加结束符
//   if(strlen(absolute_path) == 0) { // 如果绝对路径是空路径
//     strcpy(absolute_path, "/"); // 直接复制
//   }
// }


// //
// // [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// // returns the code of success, (e.g., 0 means success, fail for otherwise)
// //
// long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
//   switch (a0) {
//     case SYS_user_print:
//       return sys_user_print((const char*)a1, a2);
//     case SYS_user_exit:
//       return sys_user_exit(a1);
//     // added @lab2_2
//     case SYS_user_allocate_page:
//       return sys_user_allocate_page();
//     case SYS_user_free_page:
//       return sys_user_free_page(a1);
//     case SYS_user_fork:
//       return sys_user_fork();
//     case SYS_user_yield:
//       return sys_user_yield();
//     // added @lab4_1
//     case SYS_user_open:
//       return sys_user_open((char *)a1, a2);
//     case SYS_user_read:
//       return sys_user_read(a1, (char *)a2, a3);
//     case SYS_user_write:
//       return sys_user_write(a1, (char *)a2, a3);
//     case SYS_user_lseek:
//       return sys_user_lseek(a1, a2, a3);
//     case SYS_user_stat:
//       return sys_user_stat(a1, (struct istat *)a2);
//     case SYS_user_disk_stat:
//       return sys_user_disk_stat(a1, (struct istat *)a2);
//     case SYS_user_close:
//       return sys_user_close(a1);
//     // added @lab4_2
//     case SYS_user_opendir:
//       return sys_user_opendir((char *)a1);
//     case SYS_user_readdir:
//       return sys_user_readdir(a1, (struct dir *)a2);
//     case SYS_user_mkdir:
//       return sys_user_mkdir((char *)a1);
//     case SYS_user_closedir:
//       return sys_user_closedir(a1);
//     // added @lab4_3
//     case SYS_user_link:
//       return sys_user_link((char *)a1, (char *)a2);
//     case SYS_user_unlink:
//       return sys_user_unlink((char *)a1);
//     // ! add for lab4_challenge1
//     case SYS_user_rcwd:
//       return sys_user_rcwd(a1);
//     case SYS_user_ccwd:
//       return sys_user_ccwd(a1);
//     default:
//       panic("Unknown syscall %ld \n", a0);
//   }
// }

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
#include "vfs.h"
#include "sched.h"
#include "proc_file.h"

#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char *buf, size_t n)
{
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert(current);
  char *pa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void *)buf);
  sprint(pa);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code)
{
  sprint("User exit with code:%d.\n", code);
  // reclaim the current process, and reschedule. added @lab3_1
  free_process(current);
  schedule();
  return 0;
}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page()
{
  void *pa = alloc_page();
  uint64 va;
  // if there are previously reclaimed pages, use them first (this does not change the
  // size of the heap)
  if (current->user_heap.free_pages_count > 0)
  {
    va = current->user_heap.free_pages_address[--current->user_heap.free_pages_count];
    assert(va < current->user_heap.heap_top);
  }
  else
  {
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
uint64 sys_user_free_page(uint64 va)
{
  user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
  // add the reclaimed page to the free page list
  current->user_heap.free_pages_address[current->user_heap.free_pages_count++] = va;
  return 0;
}

//
// kerenl entry point of naive_fork
//
ssize_t sys_user_fork()
{
  sprint("User call fork.\n");
  return do_fork(current);
}

//
// kerenl entry point of yield. added @lab3_2
//
ssize_t sys_user_yield()
{
  // TODO (lab3_2): implment the syscall of yield.
  // hint: the functionality of yield is to give up the processor. therefore,
  // we should set the status of currently running process to READY, insert it in
  // the rear of ready queue, and finally, schedule a READY process to run.
  // panic( "You need to implement the yield syscall in lab3_2.\n" );

  // sprint("User call yield");
  current->status = READY;
  insert_to_ready_queue(current);
  schedule();
  return 0;
}

//
// open file
//
ssize_t sys_user_open(char *pathva, int flags)
{
  char apath[MAX_PATH_LEN * 2];
  char *pathpa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), pathva);
  rp_to_ap(pathpa, apath);
  return do_open(apath, flags);
}

//
// read file
//
ssize_t sys_user_read(int fd, char *bufva, uint64 count)
{
  int i = 0;
  while (i < count)
  { // count can be greater than page size
    uint64 addr = (uint64)bufva + i;
    uint64 pa = lookup_pa((pagetable_t)current->pagetable, addr);
    uint64 off = addr - ROUNDDOWN(addr, PGSIZE);
    uint64 len = count - i < PGSIZE - off ? count - i : PGSIZE - off;
    uint64 r = do_read(fd, (char *)pa + off, len);
    i += r;
    if (r < len)
      return i;
  }
  return count;
}

//
// write file
//
ssize_t sys_user_write(int fd, char *bufva, uint64 count)
{
  int i = 0;
  while (i < count)
  { // count can be greater than page size
    uint64 addr = (uint64)bufva + i;
    uint64 pa = lookup_pa((pagetable_t)current->pagetable, addr);
    uint64 off = addr - ROUNDDOWN(addr, PGSIZE);
    uint64 len = count - i < PGSIZE - off ? count - i : PGSIZE - off;
    uint64 r = do_write(fd, (char *)pa + off, len);
    i += r;
    if (r < len)
      return i;
  }
  return count;
}

//
// lseek file
//
ssize_t sys_user_lseek(int fd, int offset, int whence)
{
  return do_lseek(fd, offset, whence);
}

//
// read vinode
//
ssize_t sys_user_stat(int fd, struct istat *istat)
{
  struct istat *pistat = (struct istat *)user_va_to_pa((pagetable_t)(current->pagetable), istat);
  return do_stat(fd, pistat);
}

//
// read disk inode
//
ssize_t sys_user_disk_stat(int fd, struct istat *istat)
{
  struct istat *pistat = (struct istat *)user_va_to_pa((pagetable_t)(current->pagetable), istat);
  return do_disk_stat(fd, pistat);
}

//
// close file
//
ssize_t sys_user_close(int fd)
{
  return do_close(fd);
}

//
// lib call to opendir
//
ssize_t sys_user_opendir(char *pathva)
{
  char apath[MAX_PATH_LEN * 2];
  char *pathpa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), pathva);
  return do_opendir(apath);
}

//
// lib call to readdir
//
ssize_t sys_user_readdir(int fd, struct dir *vdir)
{
  struct dir *pdir = (struct dir *)user_va_to_pa((pagetable_t)(current->pagetable), vdir);
  return do_readdir(fd, pdir);
}

//
// lib call to mkdir
//
ssize_t sys_user_mkdir(char *pathva)
{
  char apath[MAX_PATH_LEN * 2];
  char *pathpa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), pathva);
  return do_mkdir(apath);
}

//
// lib call to closedir
//
ssize_t sys_user_closedir(int fd)
{
  return do_closedir(fd);
}

//
// lib call to link
//
ssize_t sys_user_link(char *vfn1, char *vfn2)
{
  char *pfn1 = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void *)vfn1);
  char *pfn2 = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void *)vfn2);
  return do_link(pfn1, pfn2);
}

//
// lib call to unlink
//
ssize_t sys_user_unlink(char *vfn)
{
  char *pfn = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void *)vfn);
  return do_unlink(pfn);
}
//
// lib call to ccwd
//
ssize_t sys_user_rcwd(uint64 path) {
  char *pa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void*)path); // 获取path的物理地址
  // sprint("jjjjjjjjjjyyyy: %s\n", current->pfiles->cwd->name);
  // uint64 pa = lookup_pa((pagetable_t)(current->pagetable), (void*)path); 
  strcpy((char*)pa, current->pfiles->cwd->name); // 将当前工作目录的路径复制到path
  return 0;
}

// 
// lib call to ccwd
//
ssize_t sys_user_ccwd(uint64 path) { // path代表路径,可能是相对路径，也可能是绝对路径

  // sprint("mzymmmmmmmmmmmmmmmmmmmmmmzzzzzzzzzzzzzzyyyyyyyyyyyyyyccwdcurrent->pfiles->cwd->name:%s\n", current->pfiles->cwd->name);
  char *pa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void*)path); // 获取path的物理地址
  // uint64 pa = lookup_pa((pagetable_t)(current->pagetable), path); // 获取path的物理地址
  char absolute_path[MAX_FILE_NAME_LEN * 2]; // 绝对路径，最终存放的必然是绝对路径
  memset(absolute_path, 0, MAX_FILE_NAME_LEN * 2); // 初始化绝对路径
  // memcpy(absolute_path, )
  rp_to_ap(pa, absolute_path); // 将相对路径转换为绝对路径
  // sprint("mzymmmmmmmmmmmmmmmmmmmmmmzzzzzzzzzzzzzzyyyyyyyyyyyyyyccwd:%s\n", absolute_path);
  // sprint("mzymmmmmmmmmmmmmmmmmmmmmmzzzzzzzzzzzzzzyyyyyyyyyyyyyyccwdcurrent->pfiles->cwd->name:%s\n", current->pfiles->cwd->name);
  strcpy(current->pfiles->cwd->name, absolute_path); // 将绝对路径复制到当前工作目录的路径
  // sprint("mzymmmmmmmmmmmmmmmmmmmmmmzzzzzzzzzzzzzzyyyyyyyyyyyyyyccwd:%s\n", current->pfiles->cwd->name);
  return 0;
}
void rp_to_ap(char *relative_path, char *absolute_path) {
  // sprint("rp:%s\n", relative_path);
  // sprint("ap:%s\n", absolute_path);
  if(relative_path == NULL || absolute_path == NULL) return;
  char * ap = absolute_path; // 绝对路径的指针
  char * cwd = current->pfiles->cwd->name; // 当前工作目录的路径
  // sprint("cwd:%s\n", cwd);
  if(strlen(relative_path) == 0) { // 如果rp是空路径
    strcpy(absolute_path, cwd); // 直接复制
    return ;
  }

  if (relative_path[0] == '/') { // 如果是根目录
    strcpy(absolute_path, relative_path); // 直接复制
    return ;
  } 
  // 重要步骤
  strcpy(absolute_path, cwd); // 先复制当前工作目录的路径
  ap += strlen(cwd); // 移动指针
  // sprint("DEBUG before while loop : %d, %d\n", ap, absolute_path);
  // sprint("DEBUG: %d, %d\n", ap, rp);
  // sprint("*****************************DEBUG the ap is : %c, %d\n", *ap, ap);
  if(*(ap-1) != '/') { // 如果当前工作目录的路径最后不是/
    *ap = '/'; // 添加/
    ap++; // 移动指针
  }
  // sprint("DEBUG the ap is : %c, %d***********************************the absult: %s\n", *(ap-1), ap, absolute_path);
  // sprint("DEBUG: %d, %d\n", ap, rp);
  char *rp = relative_path;
  // sprint("let me check rp:%s\n", rp); 
  while(*rp != 0) {
    // sprint("DEBUG before in loop : {ap:%d, abs:%d, *rp:%c}\n", ap, absolute_path, *rp);
    if(*rp == '.' && *(rp + 1) == '.') { // 如果是..
      // sprint("debug absolute_path:%d\n", absolute_path);
      // sprint("rp:%d\n", rp);
      // sprint("ap:%d\n", ap);
      // sprint("DEBUG dirst come in *******..********* : %d, %d\n", ap, absolute_path);
      if(ap == absolute_path) { // 如果是根目录
          // sprint("DEBUG: is ap %c\n", *ap);
          rp += 2; // 移动指针
          continue;
      }
      // sprint("DEBUG before while loop : %d, %d\n", ap, absolute_path);
      ap--; // 移动指针
      ap --;
      // sprint("DEBUG the ap is : %c, %d\n", *ap, ap);
      // if(ap > absolute_path) sprint("hh i am crazy ********************************************************\n");
        // sprint("debug: before while ap:%d\n", ap);
      while (ap > absolute_path && *ap != '/'){
          ap --;
          // sprint("DEBUG: while {plus one} %c\n", *ap);
      } 
      rp += 2; // 移动指针
      // sprint("rp:%s\n", rp);
      // sprint("debug: after while ap:%s\n", ap);
    }
    else if(*rp == '.' && *(rp + 1) == '.' && *(rp + 2) == '/') { // 如果是../
      if(ap == absolute_path) { // 如果是根目录
        rp += 3; // 移动指针
        continue;
      }
      ap--; // 移动指针
      while (ap > absolute_path && *ap != '/') ap --;
      rp += 3; // 移动指针
    }
    else if(*rp == '.' && *(rp + 1) == '/') { // 如果是./
      // sprint("oh i am here i mean the ./           !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      rp += 2; // 移动指针
    } 
    // else if(*rp == '/'){
    //   rp++; // 跳过 /
    // }
    else { // 如果是普通路径
      // 复制路径段到绝对路径
      while (*rp != '\0' && *rp != '/') {
          *ap = *rp;
          ap++;
          rp++;
      }
      if (*rp == '/') { // 如果路径段后有 '/'，添加 '/'
        if(ap > absolute_path && ap[-1] != '/') {
          // sprint("MMMMZZZZYYYY************** : %c\n", *ap);
          *ap = '/';
          ap++;
        }
          rp++;
      }
    }
    // sprint("ffffffffffffianlly:::::::: %s\n", rp);
  }

  if(ap >= absolute_path && ap[-1] == '/') { // 如果绝对路径最后有'/'
    ap--; // 移动指针
  }
  // sprint("DEBUG: %d, %d\n", ap, absolute_path);
  *ap = '\0'; // 添加结束符
  if(strlen(absolute_path) == 0 && *ap == '\0') { // 如果绝对路径是空路径
    strcpy(absolute_path, "/"); // 直接复制
    // sprint("hhhhhhhhhhhhhh mzy hhhhhhhhhhhhhhhhhhh fkhhhhhhhhhhhhhhhhhhhhh thu\n");
    // sprint("%s\n", absolute_path);
  }
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
  switch (a0)
  {
  case SYS_user_print:
    return sys_user_print((const char *)a1, a2);
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
  // added @lab4_1
  case SYS_user_open:
    return sys_user_open((char *)a1, a2);
  case SYS_user_read:
    return sys_user_read(a1, (char *)a2, a3);
  case SYS_user_write:
    return sys_user_write(a1, (char *)a2, a3);
    
  case SYS_user_lseek:
    return sys_user_lseek(a1, a2, a3);
  case SYS_user_stat:
    return sys_user_stat(a1, (struct istat *)a2);
  case SYS_user_disk_stat:
    return sys_user_disk_stat(a1, (struct istat *)a2);
  case SYS_user_close:
    return sys_user_close(a1);
  // added @lab4_2
  case SYS_user_opendir:
    return sys_user_opendir((char *)a1);
  case SYS_user_readdir:
    return sys_user_readdir(a1, (struct dir *)a2);
  case SYS_user_mkdir:
    return sys_user_mkdir((char *)a1);
  case SYS_user_closedir:
    return sys_user_closedir(a1);
  // added @lab4_3
  case SYS_user_link:
    return sys_user_link((char *)a1, (char *)a2);
  case SYS_user_unlink:
    return sys_user_unlink((char *)a1);
  case SYS_user_rcwd:
    return sys_user_rcwd(a1);
  case SYS_user_ccwd:
    return sys_user_ccwd(a1);
  default:
    panic("Unknown syscall %ld \n", a0);
  }
}

