#include <elf.h>
#include <stdio.h>
#include <sys/mman.h>
#include <malloc.h>
#include <stdlib.h>

#define ELF_FILE "res/gukas.elf"

const size_t PRINTF_SIZE = 0xa8;
const size_t SCANF_SIZE  = 0xac; 
const size_t ITOA_BUF    = 20;
const size_t PRINTF_BUF  = 20;
const size_t ENTRY_POINT = 0x400078;
const size_t MEM_SZ      = 0x100;
const size_t PAGESIZE    = 4096;
const size_t SCANF_BUF   = 20;

static void gen_elf_hdr (FILE* elf);
static void gen_elf_phdr(FILE* elf, size_t code_size);

static void gen_myscanf (FILE* elf);
static void gen_myprintf(FILE* elf);

size_t fileSize(FILE* file);

void gen_elf(void* bin_code, size_t size);

static void elf_empower();

