#include <common.h>
#include <elf.h>
#include <stdlib.h>

uint8_t Mag_num[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

#define shdr_table_max 40
#define phdr_table_max 40
#define sym_table_max 1000
#define str_table_max 5000
#define shstr_table_max 200

Elf64_Shdr shdr_table[shdr_table_max];
Elf64_Phdr phdr_table[phdr_table_max];
Elf64_Sym sym_table[sym_table_max];
char str_table[str_table_max];
char shstr_table[shstr_table_max];

FILE *elf_fp = NULL;

typedef struct
{
    Elf64_Addr e_entry;    /* Entry point virtual address */
    Elf64_Off e_phoff;     /* Program header table file offset */
    Elf64_Half e_phnum;    /* Program header table entry count */
    Elf64_Off e_shoff;     /* Section header table file offset */
    Elf64_Half e_shnum;    /* Section header table entry count */
    Elf64_Half e_symnum;   /* Symbol table entry count */
    Elf64_Half e_shstrndx; /* Section header string table index */
    Elf64_Half e_symdx;    /* Section symbol table index */
    Elf64_Half e_strndx;   /* Section string table index */
} ELF_helper;

typedef struct
{
    Elf64_Word st_name;  /* Symbol name (string tbl index) */
    Elf64_Addr st_value; /* Symbol value */
    Elf64_Xword st_size; /* Symbol size */
} Func;

#define funcs_max 100
Func funcs[funcs_max];
int func_len;
int call_depth;

__attribute__((used)) static void Elf64_Str_parse(ELF_helper *elf_helper)
{
    Elf64_Shdr shdr_str = shdr_table[elf_helper->e_strndx];

    assert(shdr_str.sh_size <= str_table_max);
    assert(fseek(elf_fp, shdr_str.sh_offset, SEEK_SET) >= 0);
    assert(fread(str_table, shdr_str.sh_size, 1, elf_fp) == 1);
}

__attribute__((used)) static void Elf64_Shstr_parse(ELF_helper *elf_helper)
{
    Elf64_Shdr shdr_shstr = shdr_table[elf_helper->e_shstrndx];

    assert(shdr_shstr.sh_size <= shstr_table_max);
    assert(fseek(elf_fp, shdr_shstr.sh_offset, SEEK_SET) >= 0);
    assert(fread(shstr_table, shdr_shstr.sh_size, 1, elf_fp) == 1);
    // for (int i = 0; i < elf_helper->e_shnum; i++)
    // {
    //     printf("name : %s\n", &shstr_table[shdr_table[i].sh_name]);
    // }
}

__attribute__((used)) static void Elf64_Sym_parse(ELF_helper *elf_helper)
{
    Elf64_Shdr shdr_sym = shdr_table[elf_helper->e_symdx];

    assert(shdr_sym.sh_size <= sym_table_max * sizeof(Elf64_Sym));
    assert(fseek(elf_fp, shdr_sym.sh_offset, SEEK_SET) >= 0);
    assert(fread(sym_table, shdr_sym.sh_size, 1, elf_fp) == 1);
    elf_helper->e_symnum = shdr_sym.sh_size / sizeof(Elf64_Sym);

    for (int i = 0; i < elf_helper->e_symnum; i++)
    {
        if (ELF64_ST_TYPE((sym_table[i].st_info)) == STT_FUNC)
        {
            // get func symbols!
            funcs[func_len].st_name = sym_table[i].st_name;
            funcs[func_len].st_size = sym_table[i].st_size;
            funcs[func_len++].st_value = sym_table[i].st_value;
        }
    }
}

__attribute__((used)) static void Elf64_Shdr_parse(ELF_helper *elf_helper)
{
    Elf64_Shdr shdr;
    assert(fseek(elf_fp, elf_helper->e_shoff, SEEK_SET) >= 0);

    assert(elf_helper->e_shnum <= shdr_table_max);
    // printf("e_shnum : %d\n", elf_helper->e_shnum);
    for (int i = 0; i < elf_helper->e_shnum; i++)
    {
        assert(fread(&shdr, sizeof(Elf64_Shdr), 1, elf_fp) == 1);
        if (i == 0)
            continue;
        shdr_table[i] = shdr;
        if (shdr.sh_type == SHT_STRTAB && i != elf_helper->e_shstrndx)
        {
            elf_helper->e_strndx = i;
        }
        if (shdr.sh_type == SHT_SYMTAB)
        {
            elf_helper->e_symdx = i;
        }
        // printf("name : %d, ", shdr.sh_name);
        // printf("offset %lx, ", shdr.sh_offset);
        // printf("type : %x\n", shdr.sh_type);
    }
}

__attribute__((used)) static void Elf64_Phdr_parse(ELF_helper *elf_helper)
{
    Elf64_Phdr phdr;
    assert(fseek(elf_fp, elf_helper->e_phoff, SEEK_SET) >= 0);
    assert(elf_helper->e_phnum <= phdr_table_max);

    for (int i = 0; i < elf_helper->e_phnum; i++)
    {
        assert(fread(&phdr, sizeof(Elf64_Phdr), 1, elf_fp) == 1);
        phdr_table[i] = phdr;
    }
}

__attribute__((used)) static void Elf64_Ehdr_parse(ELF_helper *elf_helper)
{
    Elf64_Ehdr ehdr;
    assert(fseek(elf_fp, 0, SEEK_SET) >= 0);
    assert(fread(&ehdr, sizeof(Elf64_Ehdr), 1, elf_fp) == 1);

    for (int i = 0; i < 4; i++)
    {
        assert(ehdr.e_ident[i] == Mag_num[i]); // 0x7f 'E' 'L' 'F'
    }
    assert(ehdr.e_ident[EI_CLASS] == ELFCLASS64);         // the 64-bit architecture.
    assert(ehdr.e_ident[EI_DATA] == ELFDATA2LSB);         // Two's complement, little-endian
    assert(ehdr.e_ident[EI_VERSION] == EV_CURRENT);       // Current version
    assert(ehdr.e_ident[EI_OSABI] == ELFOSABI_SYSV);      // UNIX System V ABI
    assert(ehdr.e_ident[EI_ABIVERSION] == ELFOSABI_SYSV); // the version of the ABI

    assert(ehdr.e_type == ET_EXEC);
    assert(ehdr.e_machine == EM_RISCV);
    assert(ehdr.e_version == EV_CURRENT);
    // printf("entry : %lx\n", ehdr.e_entry);
    // printf("phoff : %lx\n", ehdr.e_phoff);
    // printf("shoff : %lx\n", ehdr.e_shoff);
    // printf("flags : %x\n", ehdr.e_flags);
    assert(ehdr.e_ehsize == sizeof(Elf64_Ehdr));
    assert(ehdr.e_phentsize == sizeof(Elf64_Phdr));
    assert(ehdr.e_phnum < PN_XNUM); // otherwise in the initial entry (sh_info)
    assert(ehdr.e_shentsize == sizeof(Elf64_Shdr));
    assert(ehdr.e_shnum < SHN_LORESERVE); // otherwise in the initial entry (sh_size)
    assert(ehdr.e_shstrndx != SHN_UNDEF);
    // printf("shstrndx : %x\n", ehdr.e_shstrndx);
    assert(ehdr.e_shstrndx < SHN_LORESERVE); // otherwise in the initial entry (sh_link)
    elf_helper->e_entry = ehdr.e_entry;
    elf_helper->e_phnum = ehdr.e_phnum;
    elf_helper->e_phoff = ehdr.e_phoff;
    elf_helper->e_shnum = ehdr.e_shnum;
    elf_helper->e_shoff = ehdr.e_shoff;
    elf_helper->e_shstrndx = ehdr.e_shstrndx;
}

void init_ftrace(const char *elf_file)
{
    elf_fp = stdin;
    if (elf_file != NULL)
    {
        FILE *fp = fopen(elf_file, "r");
        Assert(fp, "Can not open '%s'", elf_file);
        elf_fp = fp;
    }
    Log("Read elf file %s", elf_file ? elf_file : "stdin");

    ELF_helper elf_helper;
    Elf64_Ehdr_parse(&elf_helper);  // ELF 头
    Elf64_Shdr_parse(&elf_helper);  // Section table
    Elf64_Str_parse(&elf_helper);   // String table
    Elf64_Shstr_parse(&elf_helper); // Section String table
    Elf64_Phdr_parse(&elf_helper);  // Program header table
    Elf64_Sym_parse(&elf_helper);   // Symbol table

    return;
}

// 给函数的第一个指令的地址，得到funcs数组中这个结构体的下标
static int first_pc2func_idx(vaddr_t pc)
{
    for (int i = 0; i < func_len; i++)
    {
        // printf("pc : %lx, value : %lx\n", pc, funcs[i].st_value);
        if (funcs[i].st_value == pc)
        {
            return i;
        }
    }
    return -1;
}

// 给定一个地址，找到这个地址所在的函数在funcs数组中的下标
static int section_pc2func_idx(vaddr_t pc)
{
    for (int i = 0; i < func_len; i++)
    {
        // printf("pc : %lx, value : %lx, size : %ld\n", pc, funcs[i].st_value, funcs[i].st_size);
        if (pc < funcs[i].st_value + funcs[i].st_size && pc >= funcs[i].st_value)
        {
            return i;
        }
    }
    assert(pc == 0x8000000c);
    return -1;
}

void Call_print(vaddr_t caller_pc, vaddr_t callee_pc)
{
    int caller_idx = section_pc2func_idx(caller_pc);
    int callee_idx = first_pc2func_idx(callee_pc);
    if (caller_idx == -1)
        assert(caller_pc == 0x8000000c);
    if (callee_idx == -1)// not a function, just return !
    {
        log_write("callee_pc : %lx\n", callee_pc);
        return;
    }
    // assert(callee_idx != -1);
    call_depth++;

    log_write("%lx: ", caller_pc);
    int space_cnt = call_depth - 1;
    while (space_cnt--)
    {
        log_write("  ");
    }
    if (caller_pc == 0x8000000c)
        log_write("call(__start) [%s@%lx]\n", &str_table[funcs[callee_idx].st_name], callee_pc);
    else
        log_write("call(%s) [%s@%lx]\n", &str_table[funcs[caller_idx].st_name], &str_table[funcs[callee_idx].st_name], callee_pc);
}

void Ret_print(vaddr_t ret_from_pc, vaddr_t ret_to_pc)
{
    int ret_from_idx = section_pc2func_idx(ret_from_pc);
    int ret_to_idx = section_pc2func_idx(ret_to_pc);

    log_write("%lx: ", ret_from_pc);
    int space_cnt = call_depth - 1;
    while (space_cnt--)
    {
        log_write("  ");
    }
    assert(ret_from_idx != -1);
    assert(ret_to_idx != -1);
    log_write("ret(%s) [%s@%lx]\n", &str_table[funcs[ret_from_idx].st_name], &str_table[funcs[ret_to_idx].st_name], ret_to_pc);
    call_depth--;
}