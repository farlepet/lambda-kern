#ifndef ELF_H
#define ELF_H

#include <types.h>


#define EM_NONE        0
#define EM_M32         1
#define EM_SPARC       2
#define EM_386         3
#define EM_68K         4
#define EM_88K         5
#define EM_486         6  
#define EM_860         7
#define EM_MIPS        8
#define EM_ARM         40
#define EM_IA_64       50
#define EM_X86_64      62


#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2


#if defined(ARCH_X86)
#define HOST_MACHINE EM_386
#define HOST_CLASS   ELFCLASS32
#elif defined(ARCH_ARMV7)
#define HOST_MACHINE EM_ARM
#define HOST_CLASS   ELFCLASS32
#endif




#define ELF_IDENT 0x464C457F //E,L,F,0x7F

#define ELF_IDENT0 0x7F
#define ELF_IDENT1 'E'
#define ELF_IDENT2 'L'
#define ELF_IDENT3 'F'


typedef struct
{
	uint8_t  e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} Elf32_Ehdr;

#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4





typedef struct
{
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} Elf32_Phdr;

typedef struct
{
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} Elf32_Shdr;

#define SHT_NONE          0x00
#define SHT_PROGBITS      0x01
#define SHT_SYMTAB        0x02
#define SHT_STRTAB        0x03
#define SHT_RELA          0x04
#define SHT_HASH          0x05
#define SHT_DYNAMIC       0x06
#define SHT_NOTE          0x07
#define SHT_NOBITS        0x08
#define SHT_REL           0x09
#define SHT_SHLIB         0x0A
#define SHT_DYNSYM        0x0B
#define SHT_INIT_ARRAY    0x0E
#define SHT_FINI_ARRAY    0x0F
#define SHT_PREINIT_ARRAY 0x10

/* Processor-specific section type range: */
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7FFFFFFF
/* Application-specific section type range: */
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xFFFFFFFF

extern char *sht_strings[SHT_PREINIT_ARRAY+1];


typedef struct
{
	uint32_t st_name;
	uint32_t st_value;
	uint32_t st_size;
	uint8_t  st_info;
	uint8_t  st_other;
	uint16_t st_shndx;
} Elf32_Sym;

#define ELF32_ST_BIND(i) ((i)>>4)
	#define STB_LOCAL  0
	#define STB_GLOBAL 1
	#define STB_WEAK   2
	#define STB_LOPROC 13
	#define STB_HIPROC 15

#define ELF32_ST_TYPE(i) ((i)&0xf)
	#define STT_NOTYPE  0
	#define STT_OBJECT  1
	#define STT_FUNC    2
	#define STT_SECTION 3
	#define STT_FILE    4
	#define STT_LOPROC  13
	#define STT_HIPROC  15

#define ELF32_ST_INFO(b,t) (((b)<<4+((t)&0xf))

typedef struct
{
	uint32_t r_offset;
	uint32_t r_info;
} Elf32_Rel;

typedef struct
{
	uint32_t r_offset;
	uint32_t r_info;
	int32_t  r_addend;
} Elf32_Rela;

/** Relocation r_info symbol table index. */
#define ELF32_R_SYM(i)	  ((i) >> 8)
/** Relocation r_info relcation type. */
#define ELF32_R_TYPE(i)   ((uint8_t)(i))
/** Generate r_info from symbol table index and relocation type. */
#define ELF32_R_INFO(s,t) (((s) << 8) + (uint8_t)(t))

typedef enum {
	R_386_NONE     = 0,
	R_386_32       = 1,
	R_386_PC32     = 2,
	R_386_GOT32    = 3,
	R_386_PLT32    = 4,
	R_386_COPY     = 5,
	R_386_GLOB_DAT = 6,
	R_386_JMP_SLOT = 7,
	R_386_RELATIVE = 8,
	R_386_GOTOFF   = 9,
	R_386_GOTPC    = 10,
	R_386_32PLT    = 11
} elf_386_relocation_types_e;

typedef struct
{
	uint32_t d_tag;
	uint32_t d_val;
} Elf32_Dyn;

typedef struct
{
  uint32_t a_type;
  uint32_t a_val;
} Elf32_auxv_t;

// auxv a_type values:
#define AT_NULL             0
#define AT_IGNORE           1
#define AT_EXECFD           2
#define AT_PHDR             3
#define AT_PHENT            4
#define AT_PHNUM            5
#define AT_PAGESZ           6
#define AT_BASE             7
#define AT_FLAGS            8
#define AT_ENTRY            9
#define AT_NOTELF           10
#define AT_UID              11
#define AT_EUID             12
#define AT_GID              13
#define AT_EGID             14
#define AT_PLATFORM         15
#define AT_HWCAP            16
#define AT_CLKTCK           17
#define AT_VECTOR_SIZE_BASE 19
#define AT_SECURE           23
#define AT_BASE_PLATFORM    24
#define AT_RANDOM           25
#define AT_EXECFN           31
#define AT_SYSINFO          32
#define AT_SYSINFO_EHDR     



int elf_find_section(const Elf32_Ehdr *elf, Elf32_Shdr **section, const char *section_name);

uintptr_t elf_find_data(const Elf32_Ehdr *elf, uintptr_t addr);

int load_elf(void *file, uint32_t length);


int exec_elf(void *data, uint32_t length, const char **argv, const char **envp);

#endif
