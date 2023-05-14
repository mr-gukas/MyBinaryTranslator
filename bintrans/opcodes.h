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

#define SET_RAM "\x48\xc7\xc0"
#define SET_RAM_SIZE 3
