#define PUSH_REGS "\x50\x53\x51\x52"
#define PUSH_REGS_SIZE 4

#define POP_REGS "\x5a\x59\x5b\x58"
#define POP_REGS_SIZE 4

#define PUSH_RDI "\x57"
#define PUSH_RDI_SIZE 1

#define LEA_RDI_RSP "\x48\x8d\x7c\x24\x00"
#define LEA_RDI_RSP_SIZE 5

#define MOV_RBP_RSP "\x48\x89\xe5"
#define MOV_RBP_RSP_SIZE 3

#define AND_RSP_FF "\x48\x83\xe4\xf0"
#define AND_RSP_FF_SIZE 4

#define MOV_RSP_RBP "\x48\x89\xec"
#define MOV_RSP_RBP_SIZE 3

#define MOV_RDI_RSP "\x48\x89\xe7"
#define MOV_RDI_RSP_SIZE 3

#define POP_RDI "\x5f"
#define POP_RDI_SIZE 1

#define SET_RAM "\x48\xb8" // movabs rax, imm
#define SET_RAM_SIZE 2

#define MOVABS_R10_IMM "\x49\xba"
#define MOVABS_R10_IMM_SIZE 2

#define CALL_R10 "\x41\xff\xd2"
#define CALL_R10_SIZE 3 

#define SET_PRINTF_BUF  "\x49\xbb" // movabs r11, imm
#define SET_PRINTF_BUF_SIZE 2 

#define SET_ITOA_BUF "\x49\xbc" // movabs r12, imm
#define SET_ITOA_BUF_SIZE 2

#define SET_SCANF_BUF "\x49\xbe" // movabs r14, imm
#define SET_SCANF_BUF_SIZE 2

#define SET_EXIT "\xB8\x01\x00\x00\x00\x31\xDB\xCD\x80" // mov eax, 0; xor ebx, ebx, int 0x80
#define SET_EXIT_SIZE 9



