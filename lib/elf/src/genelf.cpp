#include "../include/genelf.hpp"

void gen_elf(void* bin_code, size_t size) {
    if (!(bin_code && size)) {
        fprintf(stderr, "ER: Invalid data pointer\n");
        return;
    }
    
    FILE* elf = NULL;
    if (!(elf = fopen(ELF_FILE, "w+"))) {
        fprintf(stderr, "ER: Can't open elf file %s\n", ELF_FILE);
        return;
    }
    
    char* ram = (char*) calloc(MEM_SZ, sizeof(unsigned char));

    gen_elf_hdr (elf);
    gen_elf_phdr(elf, size);
    fwrite      (bin_code, sizeof(unsigned char), PAGESIZE, elf);
    fwrite      (ram, sizeof(unsigned char), MEM_SZ, elf);
    gen_myscanf (elf);
    gen_myprintf(elf);

    free(ram);
    fclose(elf);

    elf_empower();


}

static void gen_elf_hdr(FILE* elf) {
    Elf64_Ehdr hdr = {};
    
    hdr.e_ident[EI_MAG0]       = 0x7f;
    hdr.e_ident[EI_MAG1]       = 'E';
    hdr.e_ident[EI_MAG2]       = 'L';
    hdr.e_ident[EI_MAG3]       = 'F';
    hdr.e_ident[EI_CLASS]      = ELFCLASS64;
    hdr.e_ident[EI_DATA]       = ELFDATA2LSB;
    hdr.e_ident[EI_OSABI]      = ELFOSABI_SYSV;
    hdr.e_ident[EI_VERSION]    = EV_CURRENT;
    hdr.e_ident[EI_ABIVERSION] = EV_CURRENT;
    hdr.e_type                 = ET_EXEC;
    hdr.e_machine              = EM_X86_64;
    hdr.e_version              = EV_CURRENT;
    hdr.e_entry                = 0x400078;       //ehdr_sz = 0x40, phdr_sz = 0x38 => sum = 0x78
    hdr.e_phoff                = 0x40;
    hdr.e_shoff                = 0;
    hdr.e_ehsize               = 0x40;
    hdr.e_phentsize            = 0x38;
    hdr.e_phnum                = 1;
    
    fwrite(&hdr, sizeof(hdr), 1, elf);
}

static void gen_elf_phdr(FILE* elf, size_t code_size) {
    Elf64_Phdr phdr = {};

    phdr.p_type   = SHT_PROGBITS;
    phdr.p_flags  = SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR;
    phdr.p_offset = 0x78;
    phdr.p_vaddr  = 0x400078; 
    phdr.p_filesz = PAGESIZE + MEM_SZ + SCANF_SIZE + PRINTF_SIZE + PRINTF_BUF + ITOA_BUF;
    phdr.p_memsz  = PAGESIZE + MEM_SZ + SCANF_SIZE + PRINTF_SIZE + PRINTF_BUF + ITOA_BUF;
    phdr.p_align  = 0x1000; 

    fwrite(&phdr, sizeof(phdr), 1, elf);
}

static void gen_myscanf (FILE* elf) {
    
    FILE*  scanf_file = fopen(SCANF_BIN, "r");
    size_t scanf_sz   = fileSize(scanf_file);

    unsigned char* buf = (unsigned char*) calloc(scanf_sz + SCANF_BUF, sizeof(unsigned char));
    
    fread (buf, 1, scanf_sz, scanf_file);
    fclose(scanf_file);
    fwrite(buf, sizeof (unsigned char), scanf_sz + SCANF_BUF, elf); 

    free (buf);    
}

static void gen_myprintf (FILE* elf) {
    FILE* printf_file = fopen (PRINTF_BIN, "r");
    size_t printf_sz = fileSize (printf_file);

    unsigned char* buf = (unsigned char*) calloc(printf_sz + PRINTF_BUF + ITOA_BUF, sizeof(unsigned char));

    fread (buf, 1, printf_sz, printf_file);
    fclose(printf_file);
    fwrite(buf, sizeof (unsigned char), printf_sz + PRINTF_BUF + ITOA_BUF, elf);

    free (buf);
}

static void elf_empower() {
    char chmod_cmd[100] = {};
    sprintf(chmod_cmd, "chmod +x %s", ELF_FILE);

    system(chmod_cmd);
}

