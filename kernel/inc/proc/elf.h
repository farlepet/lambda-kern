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
#endif




#define ELF_IDENT 0x464C457F //E,L,F,0x7F

#define ELF_IDENT0 'E'
#define ELF_IDENT1 'L'
#define ELF_IDENT2 'F'
#define ELF_IDENT3 0x7F


typedef struct
{
	u8  e_ident[16];
	u16 e_type;
	u16 e_machine;
	u32 e_version;
	u32 e_entry;
	u32 e_phoff;
	u32 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} Elf32_Ehdr;

#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4





typedef struct
{
	u32 p_type;
	u32 p_offset;
	u32 p_vaddr;
	u32 p_paddr;
	u32 p_filesz;
	u32 p_memsz;
	u32 p_flags;
	u32 p_align;
} Elf32_Phdr;

typedef struct
{
	u32 sh_name;
	u32 sh_type;
	u32 sh_flags;
	u32 sh_addr;
	u32 sh_offset;
	u32 sh_size;
	u32 sh_link;
	u32 sh_info;
	u32 sh_addralign;
	u32 sh_entsize;
} Elf32_Shdr;

#define SHT_NONE     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_NOTE     7
#define SHT_NOBITS   8

typedef struct
{
	u32 st_name;
	u32 st_value;
	u32 st_size;
	u8  st_info;
	u8  st_other;
	u16 st_shndx;
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
	u32 r_offset;
	u32 r_info;
} Elf32_Rel;

typedef struct
{
	u32 r_offset;
	u32 r_info;
	s32 r_addend;
} Elf32_Rela;

typedef struct
{
	u32 d_tag;
	u32 d_val;
} Elf32_Dyn;

typedef struct
{
  u32 a_type;
  u32 a_val;
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





int load_elf(void *file, u32 length);

#endif