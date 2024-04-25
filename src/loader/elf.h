#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>

// Segment permissions
#define PF_X 0x01  // Executable
#define PF_W 0x02  // Writable
#define PF_R 0x04  // Readable

// Program header types
#define PT_NULL 0      // Unused segment
#define PT_LOAD 1      // Loadable segment
#define PT_DYNAMIC 2   // Dynamic linking information
#define PT_INTERP 3    // Program interpreter path name
#define PT_NOTE 4      // Auxiliary information
#define PT_SHLIB 5     // Reserved
#define PT_PHDR 6      // Entry for header table itself

// Section header types
#define SHT_NULL 0     // Unused section header
#define SHT_PROGBITS 1 // Information defined by the program
#define SHT_SYMTAB 2   // Symbol table
#define SHT_STRTAB 3   // String table
#define SHT_RELA 4     // Relocation entries with addends
#define SHT_HASH 5     // Symbol hash table
#define SHT_DYNAMIC 6  // Dynamic linking information
#define SHT_NOTE 7     // Notes
#define SHT_NOBITS 8   // Section occupies no space in file
#define SHT_REL 9      // Relocation entries without addends
#define SHT_SHLIB 10   // Reserved
#define SHT_DYNSYM 11  // Dynamic symbol table
#define SHT_LOPROC 12  // Processor-specific
#define SHT_HIPROC 13  // Processor-specific
#define SHT_LOUSER 14  // Lower bound of the range of indexes for application sections
#define SHT_HIUSER 15  // Upper bound of the range of indexes for application sections

// ELF file types
#define ET_NONE 0   // No file type
#define ET_REL 1    // Relocatable file
#define ET_EXEC 2   // Executable file
#define ET_DYN 3    // Shared object file
#define ET_CORE 4   // Core file

#define EI_NIDENT 16    // Size of e_ident[] in ELF header
#define EI_CLASS 4      // File class byte index
#define EI_DATA 5       // Data encoding byte index

// ELF file classes
#define ELFCLASSNONE 0  // Invalid class
#define ELFCLASS32 1    // 32-bit objects
#define ELFCLASS64 2    // 64-bit objects

// Data encoding
#define ELFDATANONE 0   // Invalid data encoding
#define ELFDATA2LSB 1   // Little endian
#define ELFDATA2MSB 2   // Big endian

#define SHN_UNDEF 0  // Undefined section index

// Basic ELF data types
typedef uint16_t elf32_half;    // Half word (16-bit)
typedef uint32_t elf32_word;    // Word (32-bit)
typedef int32_t elf32_sword;    // Signed word (32-bit)
typedef uint32_t elf32_addr;    // Unsigned program address
typedef int32_t elf32_off;      // Signed file offset

// ELF program header
struct elf32_phdr {
    elf32_word p_type;      // Type of segment
    elf32_off p_offset;     // Offset in file
    elf32_addr p_vaddr;     // Virtual address in memory
    elf32_addr p_paddr;     // Reserved
    elf32_word p_filesz;    // Size of segment in file
    elf32_word p_memsz;     // Size of segment in memory
    elf32_word p_flags;     // Segment flags
    elf32_word p_align;     // Alignment, must be power of two
} __attribute__((packed));

// ELF section header
struct elf32_shdr {
    elf32_word sh_name;      // Section name (index into string table)
    elf32_word sh_type;      // Section type
    elf32_word sh_flags;     // Section flags
    elf32_addr sh_addr;      // Address in memory
    elf32_off sh_offset;     // Offset in file
    elf32_word sh_size;      // Size of section
    elf32_word sh_link;      // Link to other section
    elf32_word sh_info;      // Additional section information
    elf32_word sh_addralign; // Section alignment
    elf32_word sh_entsize;   // Entry size if section holds table
} __attribute__((packed));

// ELF header
struct elf_header {
    unsigned char e_ident[EI_NIDENT];  // Magic number and other info
    elf32_half e_type;                 // Object file type
    elf32_half e_machine;              // Architecture
    elf32_word e_version;              // Object file version
    elf32_addr e_entry;                // Entry point virtual address
    elf32_off e_phoff;                 // Program header table file offset
    elf32_off e_shoff;                 // Section header table file offset
    elf32_word e_flags;                // Processor-specific flags
    elf32_half e_ehsize;               // ELF header size in bytes
    elf32_half e_phentsize;            // Program header table entry size
    elf32_half e_phnum;                // Program header table entry count
    elf32_half e_shentsize;            // Section header table entry size
    elf32_half e_shnum;                // Section header table entry count
    elf32_half e_shstrndx;             // Section header string table index
} __attribute__((packed));

// ELF dynamic section entry
struct elf32_dyn {
    elf32_sword d_tag;     // Dynamic entry type
    union {
        elf32_word d_val;  // Integer value
        elf32_addr d_ptr;  // Address value
    } d_un;
} __attribute__((packed));

// ELF symbol table entry. A Symbol can be a function, variable etc.
struct elf32_sym {
    elf32_word st_name;     // Symbol name (index into string table)
    elf32_addr st_value;    // Symbol value
    elf32_word st_size;     // Symbol size
    unsigned char st_info;  // Type and Binding attributes
    unsigned char st_other; // Reserved (visibility)
    elf32_half st_shndx;    // Section index
} __attribute__((packed));


void* elf_get_entry_ptr(struct elf_header *elf_header);
uint32_t elf_get_entry_(struct elf_header *elf_header);


#endif
