#ifndef _ELF_H_
#define _ELF_H_

#include "util/types.h"
#include "process.h"

#define MAX_CMDLINE_ARGS 64

// buffer size
#define MAX_SYMBOLS 512
#define MAX_STRING_LENGTH 2048


// elf header structure
typedef struct elf_header_t {
  uint32 magic;
  uint8 elf[12];
  uint16 type;      /* Object file type */
  uint16 machine;   /* Architecture */
  uint32 version;   /* Object file version */
  uint64 entry;     /* Entry point virtual address */
  uint64 phoff;     /* Program header table file offset */
  uint64 shoff;     /* Section header table file offset */
  uint32 flags;     /* Processor-specific flags */
  uint16 ehsize;    /* ELF header size in bytes */
  uint16 phentsize; /* Program header table entry size */
  uint16 phnum;     /* Program header table entry count */
  uint16 shentsize; /* Section header table entry size */
  uint16 shnum;     /* Section header table entry count */
  uint16 shstrndx;  /* Section header string table index */
} elf_header;

// Program segment header.
typedef struct elf_prog_header_t {
  uint32 type;   /* Segment type */
  uint32 flags;  /* Segment flags */
  uint64 off;    /* Segment file offset */
  uint64 vaddr;  /* Segment virtual address */
  uint64 paddr;  /* Segment physical address */
  uint64 filesz; /* Segment size in file */
  uint64 memsz;  /* Segment size in memory */
  uint64 align;  /* Segment alignment */
} elf_prog_header;

//.symtab data structure 符号表的表项
typedef struct elf_symbol_t {
    uint32 name;      // 符号名称在 .strtab 中的索引
    unsigned char info;
    unsigned char other;
    uint16 shndx;
    uint64 value;     // vitural address
    uint64 size;      // Symbol length
} elf_symbol;

//Section header table表项
typedef struct elf_section_header_t {
    uint32 name;      // 节区名称在 .shstrtab 中的索引
    uint32 type;      
    uint64 flags;
    uint64 addr;
    uint64 offset;    
    uint64 size;     
    uint32 link;
    uint32 info;
    uint64 addralign;
    uint64 entsize;
} elf_section_header;

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD 1


// 节类型常量
#define SHT_SYMTAB 2  // .symtab
#define SHT_STRTAB 3  // .strtab

// 符号类型常量
#define STT_FUNC 2    // function type


typedef enum elf_status_t {
  EL_OK = 0,

  EL_EIO,
  EL_ENOMEM,
  EL_NOTELF,
  EL_ERR,

} elf_status;

//完善后的elf_ctx
typedef struct elf_ctx_t {
  void *info;
  elf_header ehdr;
//缓冲区
  elf_symbol symbols[MAX_SYMBOLS];
  char string_table[MAX_STRING_LENGTH];
  uint64 symbol_num; // 记录读取到了多少个符号
} elf_ctx;

elf_status elf_init(elf_ctx *ctx, void *info);
elf_status elf_load(elf_ctx *ctx);

void load_bincode_from_host_elf(process *p);

#endif
