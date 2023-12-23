#include <proc.h>
#include <elf.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uint8_t __Mag_num[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

typedef struct
{
  Elf64_Addr e_entry; /* Entry point virtual address */
  Elf64_Off e_phoff;  /* Program header table file offset */
  Elf64_Half e_phnum; /* Program header table entry count */
} ELF_LoadHelper;

static void Elf_Ehdr_parser(ELF_LoadHelper *load_helper)
{
  Elf_Ehdr ehdr;
  memset((void*)&ehdr, 0, sizeof(ehdr));
  ramdisk_read((void *)&ehdr, 0, sizeof(Elf_Ehdr));

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

  load_helper->e_entry = ehdr.e_entry;
  load_helper->e_phnum = ehdr.e_phnum;
  load_helper->e_phoff = ehdr.e_phoff;
}
// 1. p_type : PT_NULL、PT_LOAD、PT_DYNAMIC、PT_INTERP、PT_NOTE、PT_SHLIB、PT_PHDR
// 2. p_flags： PF_X、PF_R、PF_W
// 3. p_offset : offset from the beginning of the file
// 4. p_vaddr : virtual address at which the first byte of the segment resides in memory
// 5. p_paddr : physical address
// 6. p_filesz : the number of bytes in the file image of the segment
// 7. p_memsz : the number of bytes in the memory image of the segment
static void Elf_Phdr_parser(ELF_LoadHelper *load_helper)
{
  printf("entry %lx e_phnum %d e_phoff %d\n", load_helper->e_entry, load_helper->e_phnum, load_helper->e_phoff);
  Elf_Phdr phdr;
  memset((void*)&phdr, 0, sizeof(phdr));

  for (int i = 0; i < load_helper->e_phnum; i++)
  {
    ramdisk_read((void *)&phdr, load_helper->e_phoff + i * sizeof(phdr), sizeof(Elf_Phdr));
    if(phdr.p_type != PT_LOAD) {
      continue;
    }
    memset((void*)phdr.p_vaddr, 0, phdr.p_memsz);
    ramdisk_read((void*)phdr.p_vaddr, phdr.p_offset, phdr.p_filesz);
    // printf("type : %lx, flags : %ld, offset : %lx, vaddr : %lx, paddr : %lx, filesz : %ld, memsz : %ld, align : %lx\n", 
    // phdr.p_type, phdr.p_flags, phdr.p_offset, phdr.p_vaddr, phdr.p_paddr, phdr.p_filesz, phdr.p_memsz, phdr.p_align);
  }
}

static uintptr_t loader(PCB *pcb, const char *filename)
{
  ELF_LoadHelper elf_load_helper;

  Elf_Ehdr_parser(&elf_load_helper);
  Elf_Phdr_parser(&elf_load_helper);

  // TODO();
  return elf_load_helper.e_entry;
}

void naive_uload(PCB *pcb, const char *filename)
{
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
