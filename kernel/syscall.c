/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>
#include "elf.h"
#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"

#include "spike_interface/spike_utils.h"
extern elf_ctx elfloader;
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}


/*ssize_t sys_user_backtrace(uint64 depth) {
  //回溯的起点
  uint64 fp = current->trapframe->regs.s0;

  uint64 current_depth = 0;
  uint64 ra = 0;

  //调试检测elf.c
  sprint("DEBUG: symbol_num = %d\n", elfloader.symbol_num);
  
  while (current_depth < depth && fp != 0) {
    //sd ra, 8(sp); addi s0, sp, 16
    ra = *(uint64*)(fp - 8); //返回地址

    int string_index = -1;

    for (int i = 0; i < elfloader.symbol_num; i++) {
      elf_symbol symbol = elfloader.symbols[i];

      if ((symbol.info & 0xf) != STT_FUNC) continue;

      //find out
      if (ra >= symbol.value && ra < (symbol.value + symbol.size)) {
        string_index = i;
        break; //找到立刻停止
      }
    }
    if (string_index != -1) {
      //根据索引在符号表中找到对应的symbol，取其name字段在字符串表中寻找；
      char *funcname = elfloader.string_table + elfloader.symbols[string_index].name;

      sprint("%s\n", funcname);

      //main
      if (strcmp(funcname, "main") == 0) break;
    } else {
      sprint("0x%lx\n", ra);
    }

    current_depth++;
    fp = *(uint64*)(fp - 16);
  }
  return 0;
}*/

//correct plus
ssize_t sys_user_backtrace(uint64 depth) {
    uint64 fp = current->trapframe->regs.s0; //栈底指针
    //uint64 sp = current->trapframe->regs.sp; // 真实的栈底 sp

    //sprint("DEBUG VERIFY:\n");
    //sprint("regs.s0 = 0x%lx\n", fp);
    //sprint("regs.sp = 0x%lx\n", sp);
    
    //if (fp == sp) {
    //    sprint("Conclusion: s0 == sp (Pointing to Bottom)\n");
    //} else if (fp == sp + 16) {
    //    sprint("Conclusion: s0 == sp + 16 (Pointing to Top)\n");
    //} else {
    //    sprint("Conclusion: s0 = sp + %ld (Unknown Layout)\n", (long)fp - (long)sp);
    //}
    uint64 sp = 0; 
    uint64 ra = 0;
    uint64 current_depth = 0;

    //sprint("back trace the user app in the following:\n");
    while (current_depth < depth && fp != 0) {
        
      //被调用函数的栈底为调用函数的栈顶
        sp = *(uint64*)fp; 
      //fp+8即sp+8
        ra = *(uint64*)(fp + 8);

        int best_index = -1;
        // 查找最符合 ra 的符号
        for (int i = 0; i < elfloader.symbol_num; i++) {
            elf_symbol symbol = elfloader.symbols[i];
            
            // 只找函数
            if ((symbol.info & 0xf) != STT_FUNC) continue;
            
            // 判断 ra 是否在函数范围内 [value, value + size)
            if (ra >= symbol.value) {
                 if (ra < symbol.value + symbol.size || symbol.size == 0) {
                     best_index = i;
                     break; 
                 }
            }
        }

        if (best_index != -1) {
            char *name = elfloader.string_table + elfloader.symbols[best_index].name;
            sprint("%s\n", name);
            if (strcmp(name, "main") == 0) break;
        } else {
            sprint("0x%lx\n", ra);
        }
        //分析栈帧结构可知所有函数的栈帧的大小均为16B
        fp = sp - 16;
        
        current_depth++;
    }

    return 0;
}





//correct 
/*ssize_t sys_user_backtrace(uint64 depth) {
  uint64 current_depth = 0;
  uint64 ra = 0; // 返回地址,叶子函数没有返回地址
  uint64 sp = current->trapframe->regs.sp; // 获取当前栈指针
  uint64 fp = current->trapframe->regs.s0; // 根据寄存器组获取保存的栈指针
  int i;
  while (current_depth < depth)
  {
    current_depth ++;
    sp = *(uint64*)fp; // 获取上一个栈指针
    ra = *(uint64*)(fp + 8); // 获取返回地址
    fp = sp - 16; // 获取上一个栈帧指针
    int string_index = -1; // 表示所获得的字符串索引
    uint64 pre_ra = 0; // 上一个返回地址
    // sprint("DEBUG: %d\n", elfloader.symbol_num);
    for(i = 0; i < elfloader.symbol_num; i ++){
      elf_symbol symbol = elfloader.symbols[i];
      if ((symbol.info & 0xf) != STT_FUNC) continue; // 不是函数符号，continue (函数符号在符号表中的type为2)
      // value表示符号的地址,这里的判断筛选出当前符号
      if(symbol.value <= ra && symbol.value > pre_ra){
        pre_ra = symbol.value; // 更新pre_ra
        string_index = i; // 对应符号表中的第i个符号
        // break;
      }
    }
    // elf_symbol symbol = elfloader.symbols[string_index];
    if(string_index != -1){
      // sprint()
      sprint("%s\n", elfloader.string_table + elfloader.symbols[string_index].name);
      if(strcmp("main", elfloader.string_table + elfloader.symbols[string_index].name) == 0) break;
    }
    else{
      sprint("Backtrace %d: unknown\n", current_depth);
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
    case SYS_user_backtrace:
      return sys_user_backtrace(a1);
    default:
      panic("Unknown syscall %ld \n", a0); 
  } 
}
