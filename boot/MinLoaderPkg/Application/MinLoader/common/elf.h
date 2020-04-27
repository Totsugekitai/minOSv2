#include <stdint.h>

// Data Types
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef uint64_t Elf64_Sxword;
typedef uint8_t  unsigned_char;

// File Header
typedef struct Elf64_Ehdr {
    unsigned_char   e_ident[16];        // ELF identification
    Elf64_Half      e_type;             // Object file type
    Elf64_Half      e_machine;          // Machine type
    Elf64_Word      e_version;          // Object file version
    Elf64_Addr      e_entry;            // Entry point Address
    Elf64_Off       e_phoff;            // Program header offfset
    Elf64_Off       e_shoff;            // Section header offfset
    Elf64_Word      e_flags;            // Processor-specific flags
    Elf64_Half      e_ehsize;           // ELF header size
    Elf64_Half      e_phentsize;        // Size of program header entry
    Elf64_Half      e_phnum;            // Number of program header entries
    Elf64_Half      e_shentsize;        // Size of section header entry
    Elf64_Half      e_shnum;            // Number of section header entries
    Elf64_Half      e_shstrndx;         // Section name string table index
} Elf64_Ehdr;

// Section Header
typedef struct Elf64_Shdr {
    Elf64_Word      sh_name;
    Elf64_Word      sh_type;
    Elf64_Xword     sh_flags;
    Elf64_Addr      sh_addr;
    Elf64_Off       sh_offset;
    Elf64_Xword     sh_size;
    Elf64_Word      sh_link;
    Elf64_Word      sh_info;
    Elf64_Xword     sh_addralign;
    Elf64_Xword     sh_entsize;
} Elf64_Shdr;

