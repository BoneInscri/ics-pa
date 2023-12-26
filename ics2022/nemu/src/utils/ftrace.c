#include <common.h>
#include <elf.h>
#include <stdlib.h>

uint8_t Mag_num[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

#define shdr_table_max 15
// #define phdr_table_max 10
#define sym_table_max 500
#define str_table_max 2000
// #define shstr_table_max 200
#define funcs_max 200

typedef struct
{
    Elf64_Word st_name;  /* Symbol name (string tbl index) */
    Elf64_Addr st_value; /* Symbol value */
    Elf64_Xword st_size; /* Symbol size */
    int idx;             // which elf ?
} Func;

typedef struct
{
    Elf64_Addr e_entry; /* Entry point virtual address */

    Elf64_Off e_phoff;  /* Program header table file offset */
    Elf64_Half e_phnum; /* Program header table entry count */

    Elf64_Off e_shoff;   /* Section header table file offset */
    Elf64_Half e_shnum;  /* Section header table entry count */
    Elf64_Half e_symnum; /* Symbol table entry count */
    // Elf64_Half e_shstrndx; /* Section header string table index */
    Elf64_Half e_symdx;  /* Section symbol table index */
    Elf64_Half e_strndx; /* Section string table index */

    Elf64_Shdr shdr_table[shdr_table_max];
    Elf64_Sym sym_table[sym_table_max];
    // char shstr_table[shstr_table_max];
    FILE *elf_fp;
} ELF_helper;

#define ELF_CNT 2
char str_table[ELF_CNT][str_table_max];
int elf_cnt;
Func funcs[funcs_max];
int func_len;
int call_depth;

__attribute__((used)) static void Elf64_Str_parse(ELF_helper *elf_helper)
{
    Elf64_Shdr shdr_str = elf_helper->shdr_table[elf_helper->e_strndx];
    // printf("strndx %x offset : %lx size : %lx\n", elf_helper->e_strndx, shdr_str.sh_offset, shdr_str.sh_size);
    assert(shdr_str.sh_size <= str_table_max);
    assert(fseek(elf_helper->elf_fp, shdr_str.sh_offset, SEEK_SET) >= 0);
    printf("elf_cnt : %d\n", elf_cnt);
    assert(fread(&str_table[elf_cnt], shdr_str.sh_size, 1, elf_helper->elf_fp) == 1);
}

// __attribute__((used)) static void Elf64_Shstr_parse(ELF_helper *elf_helper)
// {
//     Elf64_Shdr shdr_shstr = elf_helper->shdr_table[elf_helper->e_shstrndx];

//     assert(shdr_shstr.sh_size <= shstr_table_max);
//     assert(fseek(elf_helper->elf_fp, shdr_shstr.sh_offset, SEEK_SET) >= 0);
//     assert(fread(elf_helper->shstr_table, shdr_shstr.sh_size, 1, elf_helper->elf_fp) == 1);
// }

__attribute__((used)) static void Elf64_Sym_parse(ELF_helper *elf_helper)
{
    Elf64_Shdr shdr_sym = elf_helper->shdr_table[elf_helper->e_symdx];

    assert(shdr_sym.sh_size <= sym_table_max * sizeof(Elf64_Sym));
    assert(fseek(elf_helper->elf_fp, shdr_sym.sh_offset, SEEK_SET) >= 0);
    assert(fread(elf_helper->sym_table, shdr_sym.sh_size, 1, elf_helper->elf_fp) == 1);
    elf_helper->e_symnum = shdr_sym.sh_size / sizeof(Elf64_Sym);

    for (int i = 0; i < elf_helper->e_symnum; i++)
    {
        if (!strcmp(&str_table[elf_cnt][elf_helper->sym_table[i].st_name], "_start") || ELF64_ST_TYPE((elf_helper->sym_table[i].st_info)) == STT_FUNC)
        {
            // get func symbols!
            funcs[func_len].idx = elf_cnt;
            funcs[func_len].st_name = elf_helper->sym_table[i].st_name;
            funcs[func_len].st_size = elf_helper->sym_table[i].st_size;
            funcs[func_len++].st_value = elf_helper->sym_table[i].st_value;
            assert(func_len <= funcs_max);
        } 
        // else {
        //     printf("idx : %d, value : %lx, size : %ld, name : %s\n", elf_cnt, elf_helper->sym_table[i].st_value, elf_helper->sym_table[i].st_size, &str_table[elf_cnt][elf_helper->sym_table[i].st_name]);    
        // }
    }

}

__attribute__((used)) static void Elf64_Shdr_parse(ELF_helper *elf_helper)
{
    Elf64_Shdr shdr;
    assert(fseek(elf_helper->elf_fp, elf_helper->e_shoff, SEEK_SET) >= 0);

    assert(elf_helper->e_shnum <= shdr_table_max);
    int first = 1;
    // printf("e_shnum : %d\n", elf_helper->e_shnum);
    for (int i = 0; i < elf_helper->e_shnum; i++)
    {
        assert(fread(&shdr, sizeof(Elf64_Shdr), 1, elf_helper->elf_fp) == 1);
        if (i == 0)
            continue;
        elf_helper->shdr_table[i] = shdr;
        if (shdr.sh_type == SHT_SYMTAB)
        {
            elf_helper->e_symdx = i;
        }

        if (shdr.sh_type == SHT_STRTAB)
        {
            if (first)
            {
                elf_helper->e_strndx = i;
                first = 0;
            }
            // else {
            //     elf_helper->e_shstrndx = i;
            // }
        }

        // printf("name : %d, ", shdr.sh_name);
        // printf("offset %lx, ", shdr.sh_offset);
        // printf("type : %x\n", shdr.sh_type);
    }
}

// __attribute__((used)) static void Elf64_Phdr_parse(ELF_helper *elf_helper)
// {
//     Elf64_Phdr phdr;
//     assert(fseek(elf_helper->elf_fp, elf_helper->e_phoff, SEEK_SET) >= 0);
//     assert(elf_helper->e_phnum <= phdr_table_max);

//     for (int i = 0; i < elf_helper->e_phnum; i++)
//     {
//         assert(fread(&phdr, sizeof(Elf64_Phdr), 1, elf_helper->elf_fp) == 1);
//         elf_helper->phdr_table[i] = phdr;
//     }
// }

__attribute__((used)) static void Elf64_Ehdr_parse(ELF_helper *elf_helper)
{
    Elf64_Ehdr ehdr;
    assert(fseek(elf_helper->elf_fp, 0, SEEK_SET) >= 0);
    assert(fread(&ehdr, sizeof(Elf64_Ehdr), 1, elf_helper->elf_fp) == 1);

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
    assert(ehdr.e_shstrndx < SHN_LORESERVE); // otherwise in the initial entry (sh_link)
    // printf("shstrndx : %x\n", ehdr.e_shstrndx);
    elf_helper->e_entry = ehdr.e_entry;
    // elf_helper->e_phnum = ehdr.e_phnum;
    // elf_helper->e_phoff = ehdr.e_phoff;
    elf_helper->e_shnum = ehdr.e_shnum;
    elf_helper->e_shoff = ehdr.e_shoff;
    // elf_helper->e_shstrndx = ehdr.e_shstrndx;
}

static ELF_helper elf_helper;
__attribute__((used)) void init_ftrace(const char *elf_file)
{

    memset((void *)&elf_helper, 0, sizeof(elf_helper));
    printf("size %ld \n", sizeof(ELF_helper));

    elf_helper.elf_fp = stdin;
    if (elf_file != NULL)
    {
        FILE *fp = fopen(elf_file, "r");
        Assert(fp, "Can not open '%s'", elf_file);
        elf_helper.elf_fp = fp;
    }
    Log("Read elf file %s", elf_file ? elf_file : "stdin");
    Elf64_Ehdr_parse(&elf_helper); // ELF 头
    Elf64_Shdr_parse(&elf_helper); // Section table
    Elf64_Str_parse(&elf_helper);  // String table
    // Elf64_Shstr_parse(elf_helper); // Section String table
    Elf64_Sym_parse(&elf_helper); // Symbol table

    elf_cnt++;
    assert(elf_cnt <= ELF_CNT);
    return;
}

// 给函数的第一个指令的地址，得到funcs数组中这个结构体的下标
static int first_pc2func_idx(vaddr_t pc)
{
    for (int i = 0; i < func_len; i++)
    {
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
        // int idx = funcs[i].idx;
        // printf("idx : %d, pc : %lx, value : %lx, size : %ld, name : %s\n", idx, pc, funcs[i].st_value, funcs[i].st_size, &str_table[idx][funcs[i].st_name]);
        if (pc >= funcs[i].st_value && pc < funcs[i].st_value + funcs[i].st_size)
        {
            return i;
        }
    }
    // panic("pause");
    return -1;
}

__attribute__((used)) static void print_space()
{
    int space_cnt = call_depth - 1;
    if(space_cnt<0) {
        return;
    }
    while (space_cnt--)
    {
        my_log("  ");
    }
}

__attribute__((used)) void Call_print(vaddr_t caller_pc, vaddr_t callee_pc)
{
    if(callee_pc == 0x83004f98) {
        panic("pause");
    }
    int caller_idx = section_pc2func_idx(caller_pc);
    int callee_idx = first_pc2func_idx(callee_pc);

    int flag = 0;
    if (caller_idx == -1 && callee_idx != -1)
    {
        flag = 1;
    }

    // assert(caller_idx != -1);

    assert(callee_idx != -1);

    call_depth++;

    my_log("%lx: ", caller_pc);
    print_space();

    if (flag)
    {
        int elf_idx = funcs[callee_idx].idx;

        // if(elf_idx == 1)         
        // my_log("call(__start) [%s@%lx]\n", &(str_table[elf_idx][funcs[callee_idx].st_name]), callee_pc);
        my_log("call [%s@%lx]\n", &(str_table[elf_idx][funcs[callee_idx].st_name]), callee_pc);
    }
    else
    {
        // int elf_idx_1 = funcs[caller_idx].idx;
        int elf_idx_2 = funcs[callee_idx].idx;

        // if(elf_idx_2 == 1 && callee_pc == 0x830016e4) {
        //     printf("=====================%lx====================\n", funcs[callee_idx].st_value);
        //     printf("call(%s) [%s@%lx]\n", &(str_table[elf_idx_1][funcs[caller_idx].st_name]), &(str_table[elf_idx_2][funcs[callee_idx].st_name]), callee_pc);
        //     panic("pause");
        // }
        my_log("call [%s@%lx]\n", &(str_table[elf_idx_2][funcs[callee_idx].st_name]), callee_pc);
        // my_log("call(%s) [%s@%lx]\n", &(str_table[elf_idx_1][funcs[caller_idx].st_name]), &(str_table[elf_idx_2][funcs[callee_idx].st_name]), callee_pc);
    }
}

__attribute__((used)) void Ret_print(vaddr_t ret_from_pc, vaddr_t ret_to_pc)
{
    int ret_from_idx = section_pc2func_idx(ret_from_pc);
    int ret_to_idx = section_pc2func_idx(ret_to_pc);

    if (ret_from_idx == -1)
    {
        panic("Ret_print : ret_from_idx invalid");
    }
    if (ret_to_idx == -1)
    {
        printf("ret_to_pc : %lx\n", ret_to_pc);
        return ;
    }

    int elf_idx_1 = funcs[ret_from_idx].idx;
    // int elf_idx_2 = funcs[ret_to_idx].idx;

    my_log("%lx: ", ret_from_pc);
    print_space();

    // if(ret_from_idx == 1 && ret_to_idx == 1)
    // my_log("ret(%s) [%s@%lx]\n", &(str_table[elf_idx_1][funcs[ret_from_idx].st_name]), &(str_table[elf_idx_2][funcs[ret_to_idx].st_name]), ret_to_pc);
    my_log("[%s@%lx] ret\n", &(str_table[elf_idx_1][funcs[ret_from_idx].st_name]), ret_from_pc);
    // my_log("ret [%s@%lx]\n", &(str_table[elf_idx_2][funcs[ret_to_idx].st_name]), ret_to_pc);
    call_depth--;
}