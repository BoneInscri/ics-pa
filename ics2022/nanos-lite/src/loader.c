#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym Elf64_Sym
#define Elf_ST_TYPE ELF64_ST_TYPE
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym Elf32_Sym
#define Elf_ST_TYPE ELF32_ST_TYPE
#endif

#if defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_RISCV64__)
#define EXPECT_TYPE EM_RISCV
#else
#error Unsupported ISA
#endif

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uint8_t __Mag_num[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

#define str_table_max 3000
#define shdr_table_max 15
typedef struct
{
  Elf64_Addr e_entry; /* Entry point virtual address */

  Elf64_Off e_phoff;  /* Program header table file offset */
  Elf64_Half e_phnum; /* Program header table entry count */

  Elf64_Off e_shoff;  /* Section header table file offset */
  Elf64_Half e_shnum; /* Section header table entry count */

  Elf64_Half e_symdx;    /* Section symbol table index */
  Elf64_Half e_symnum;   /* Symbol table entry count */
  Elf64_Half e_strndx;   /* Section string table index */
  Elf64_Half e_shstrndx; /* Section header string table index */
  Elf64_Shdr shdr_table[shdr_table_max];
  char str_table[str_table_max];

  int fd;
} ELF_LoadHelper;

// elf header
static void Elf_Ehdr_parser(ELF_LoadHelper *load_helper)
{
  Elf_Ehdr ehdr;
  memset((void *)&ehdr, 0, sizeof(ehdr));
  // ramdisk_read((void *)&ehdr, 0, sizeof(Elf_Ehdr));
  fs_lseek(load_helper->fd, 0, SEEK_SET);
  fs_read(load_helper->fd, (void*)&ehdr, sizeof(Elf_Ehdr));

  for (int i = 0; i < 4; i++)
  {
    assert(ehdr.e_ident[i] == __Mag_num[i]); // 0x7f 'E' 'L' 'F'
  }
  assert(ehdr.e_ident[EI_CLASS] == ELFCLASS64);         // the 64-bit architecture.
  assert(ehdr.e_ident[EI_DATA] == ELFDATA2LSB);         // Two's complement, little-endian
  assert(ehdr.e_ident[EI_VERSION] == EV_CURRENT);       // Current version
  assert(ehdr.e_ident[EI_OSABI] == ELFOSABI_SYSV);      // UNIX System V ABI
  assert(ehdr.e_ident[EI_ABIVERSION] == ELFOSABI_SYSV); // the version of the ABI

  assert(ehdr.e_type == ET_EXEC);
  assert(ehdr.e_machine == EM_RISCV);
  assert(ehdr.e_version == EV_CURRENT);

  assert(ehdr.e_ehsize == sizeof(Elf_Ehdr));
  assert(ehdr.e_phentsize == sizeof(Elf_Phdr));
  assert(ehdr.e_phnum < PN_XNUM); // otherwise in the initial entry (sh_info)

  assert(ehdr.e_shentsize == sizeof(Elf64_Shdr));
  assert(ehdr.e_shnum < SHN_LORESERVE); // otherwise in the initial entry (sh_size)

  load_helper->e_entry = ehdr.e_entry;

  load_helper->e_phnum = ehdr.e_phnum;
  load_helper->e_phoff = ehdr.e_phoff;

  load_helper->e_shoff = ehdr.e_shoff;
  load_helper->e_shnum = ehdr.e_shnum;

  load_helper->e_shstrndx = ehdr.e_shstrndx;
}
// 1. p_type : PT_NULL、PT_LOAD、PT_DYNAMIC、PT_INTERP、PT_NOTE、PT_SHLIB、PT_PHDR
// 2. p_flags： PF_X、PF_R、PF_W
// 3. p_offset : offset from the beginning of the file
// 4. p_vaddr : virtual address at which the first byte of the segment resides in memory
// 5. p_paddr : physical address
// 6. p_filesz : the number of bytes in the file image of the segment
// 7. p_memsz : the number of bytes in the memory image of the segment

// program table
static void Elf_Phdr_parser(ELF_LoadHelper *load_helper)
{
  printf("entry %lx e_phnum %d e_phoff %d\n", load_helper->e_entry, load_helper->e_phnum, load_helper->e_phoff);
  Elf_Phdr phdr;
  memset((void *)&phdr, 0, sizeof(phdr));
  int elem_size = sizeof(phdr);

  for (int i = 0; i < load_helper->e_phnum; i++)
  {
    // ramdisk_read((void *)&phdr, load_helper->e_phoff + i * elem_size, elem_size);
    fs_lseek(load_helper->fd, load_helper->e_phoff + i * elem_size, SEEK_SET);
    fs_read(load_helper->fd, (void*)&phdr, sizeof(Elf_Ehdr));

    if (phdr.p_type != PT_LOAD)
    {
      continue;
    }
    // printf("p_vaddr : %lx p_filesz : %lx p_memsz : %lx p_offset : %lx\n", phdr.p_vaddr, phdr.p_filesz, phdr.p_memsz, phdr.p_offset);
    memset((void *)phdr.p_vaddr, 0, phdr.p_memsz);
    fs_lseek(load_helper->fd, phdr.p_offset, SEEK_SET);
    fs_read(load_helper->fd, (void*)phdr.p_vaddr, phdr.p_filesz);
    // ramdisk_read((void*)phdr.p_vaddr, );
    // ramdisk_read((void *)phdr.p_vaddr, phdr.p_offset, phdr.p_filesz);
    // printf("type : %lx, flags : %ld, offset : %lx, vaddr : %lx, paddr : %lx, filesz : %ld, memsz : %ld, align : %lx\n",
    // phdr.p_type, phdr.p_flags, phdr.p_offset, phdr.p_vaddr, phdr.p_paddr, phdr.p_filesz, phdr.p_memsz, phdr.p_align);
  }
}

// section table
static void Elf_Shdr_parser(ELF_LoadHelper *load_helper)
{
  Elf_Shdr shdr;
  memset((void *)&shdr, 0, sizeof(shdr));
  int elem_size = sizeof(shdr);

  assert(load_helper->e_shnum <= shdr_table_max);

  int first = 1;
  for (int i = 0; i < load_helper->e_shnum; i++)
  {
    // ramdisk_read((void *)&shdr, load_helper->e_shoff + i * elem_size, elem_size);
    fs_lseek(load_helper->fd, load_helper->e_shoff + i * elem_size, SEEK_SET);
    fs_read(load_helper->fd, (void*)&shdr, elem_size);

    load_helper->shdr_table[i] = shdr;
    if (shdr.sh_type == SHT_SYMTAB)
    {
      load_helper->e_symdx = i;
    }

    if (shdr.sh_type == SHT_STRTAB && first)
    {
      load_helper->e_strndx = i;
      first = 0;
    }
  }
}

// string table
static void Elf_Str_parse(ELF_LoadHelper *loader_helper)
{
  Elf64_Shdr shdr_str = loader_helper->shdr_table[loader_helper->e_strndx];
  assert(shdr_str.sh_size <= str_table_max);
  // ramdisk_read((void *)&(loader_helper->str_table), shdr_str.sh_offset, shdr_str.sh_size);
  fs_lseek(loader_helper->fd, shdr_str.sh_offset, SEEK_SET);
  fs_read(loader_helper->fd, (void*)&(loader_helper->str_table), shdr_str.sh_size);
}

// symbol table
static void Elf_Sym_parser(ELF_LoadHelper *load_helper)
{
  Elf_Shdr shdr_sym = load_helper->shdr_table[load_helper->e_symdx];
  int elem_size = sizeof(Elf_Sym);
  load_helper->e_symnum = shdr_sym.sh_size / elem_size;

  Elf_Sym sym;

  for (int i = 0; i < load_helper->e_symnum; i++)
  {
    // ramdisk_read((void *)&sym, shdr_sym.sh_offset + i * elem_size, elem_size);
    fs_lseek(load_helper->fd, shdr_sym.sh_offset + i * elem_size, SEEK_SET);
    fs_read(load_helper->fd, (void*)&sym, elem_size);

    if (Elf_ST_TYPE((sym.st_info)) == STT_NOTYPE)
    {
      char *name = &(load_helper->str_table[sym.st_name]);
      if (!strcmp(name, "_end"))
      {
        extern uintptr_t brk;
        brk = sym.st_value;
      }
    }
  }
}

static ELF_LoadHelper elf_load_helper;


// load program
static uintptr_t loader(PCB *pcb, const char *filename)
{
  assert(filename);
  int fd = fs_open(filename, 0, 0);
  elf_load_helper.fd = fd;

  printf("ELF_LoadHelper sizeof :  %d\n", sizeof(ELF_LoadHelper));
  Elf_Ehdr_parser(&elf_load_helper);
  Elf_Phdr_parser(&elf_load_helper);
  Elf_Shdr_parser(&elf_load_helper);

  Elf_Str_parse(&elf_load_helper);
  Elf_Sym_parser(&elf_load_helper);

  return elf_load_helper.e_entry;
}

void naive_uload(PCB *pcb, const char *filename)
{
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
