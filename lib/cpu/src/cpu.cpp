#include "../include/cpu.h"
#include <chrono>

int main(int argc, const char* argv[])
{   
    if (argc != 2)
    {
        fprintf(stderr, ">>>Incorrect command line arguments. Exiting the program....\n");
        abort();
    }   

    FILE* binary = NULL;
    if ((binary = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, ">>>Incorrect filename: %s\n", argv[1]);
        abort();
    }
    
    Cpu_t cpu = {};

    $$(CpuCtor(&cpu, binary));

#if TIME_MEASURE_MODE
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 100; ++i)
#endif
        RunCpu(&cpu);
#if TIME_MEASURE_MODE
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    printf ("Время выполнения, мс: %lu\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
#endif


    $$(CpuDtor(&cpu, binary));

    return 0;
}

int CpuCtor(Cpu_t* cpu, FILE* binary)
{
    StackCtor(&cpu->stk,    0);
    StackCtor(&cpu->retStk, 0);
    
    CmdInfo_t binInfo = {};

    fread(&binInfo, sizeof (CmdInfo_t), 1, binary);
    
    if (binInfo.sign != BASIC_SIGN || binInfo.vers != BASIC_VERS)
    {
        return 1;
    }
    
    cpu->size   =  binInfo.filesize;
    cpu->cmdArr = (unsigned char*) calloc (1, cpu->size + 2);
    
    if (cpu->cmdArr == NULL)
        return 1;

    fread(cpu->cmdArr, sizeof(char), binInfo.filesize + 1, binary);

    return 0;
}

int CpuDtor(Cpu_t* cpu, FILE* binary)
{
    StackDtor(&cpu->stk);
    StackDtor(&cpu->retStk);
    free     (cpu->cmdArr);
    fclose   (binary);

    binary = nullptr;

    return 0;
}

int RunCpu(Cpu_t* cpu)
{   
    int ip = 0;
    
    while (ip <= cpu->size)
    {
        int curCmd = *(cpu->cmdArr + ip);

#define DEF_CMD(name, num, arg, code) \
    case CMD_##name:                   \
        code                            \
        break;

#define DEF_JMP(name, num, sign)         \
    case JMP_##name:                      \
    {                                      \
        INDEX_UP;                           \
        if ((curCmd & ONLY_CMD) == JMP_JMP)  \
        {                                     \
            $$(GET_JMP_ARG);                   \
        }                                       \
        else if ((curCmd & ONLY_CMD) == JMP_CALL)\
        {                                         \
            PUSH_RET((ip + sizeof(int) - 1));      \
            $$(GET_JMP_ARG);                        \
        }                                            \
		else if ((curCmd & ONLY_CMD) == JMP_JE)\
		{\
			VAR NUM2 = POP;\
            VAR NUM1 = POP;                              \
			if (!DoubleCmp(NUM1, NUM2))\
			{\
				$$(GET_JMP_ARG);\
			}\
			else\
				ip += sizeof(int) - 1;\
		}\
        else                                          \
        {                                              \
            VAR NUM2 = POP;                             \
            VAR NUM1 = POP;                              \
                                                          \
            if ((NUM1) sign (NUM2))                        \
            {                                               \
                $$(GET_JMP_ARG);                             \
            }                                                 \
            else                                               \
                ip += sizeof(int) - 1;                          \
        }                                                        \
        break;                                                    \
	}
    

        switch (curCmd & ONLY_CMD)
        {
            #include "../include/cmd.h"

            default:
                return 1;
        }

        ++ip;
    }

#undef DEF_CMD
#undef DEF_JMP

    return 0;
} 

arg_t GetPushArg(int command, int* ip, Cpu_t* cpu)
{
    arg_t  arg    = 0;
    arg_t  value  = 0;
    int    curReg = 0; 
    short  ipCtrl = 0;

    if (command & ARG_IMMED)
    {	
		if (command & ARG_MEM)
		{
			int index = 0;
			memcpy(&index, cpu->cmdArr + *ip, sizeof(int));
			arg += index;
		 
			*ip += sizeof(int);
		}
		else
		{
			memcpy(&value, cpu->cmdArr + *ip, sizeof(arg_t));
			arg += value;
		 
			*ip += sizeof(arg_t);
		}
    }

    if (command & ARG_REG)
    {
		memcpy(&curReg, cpu->cmdArr + *ip, sizeof(int));
		arg += *(cpu->regs + curReg);
       
       *ip += sizeof(int);
    }

    if (command & ARG_MEM)
    {
        if ((int)arg > MAX_RAM_SIZE)
        {
            return POISON_ARG;
        }

        else
		{
            arg = cpu->RAM[(int) arg];
		}

    }
    
    if ((command & (ARG_IMMED | ARG_REG | ARG_MEM)) == 0)
    {
        return POISON_ARG;
    }

    *ip -= 1;
	
    return arg;
}

arg_t* GetPopArg(int command, int* ip, Cpu_t* cpu)
{
    int arg    = 0; 
    int    value  = 0;
    int    curReg = 0; 

    
    if (command & ARG_MEM)
    {
        if (command & ARG_IMMED)
        {   
            memcpy(&value, cpu->cmdArr + *ip, sizeof(int));
            *ip += sizeof(int);
            
            arg += (int) value;
        }

        if (command & ARG_REG)
        {
            memcpy(&curReg, cpu->cmdArr + *ip, sizeof(int));
            *ip += sizeof(int);

            arg += cpu->regs[curReg];
        }

        if (arg >= MAX_RAM_SIZE)
        {
            return NULL;
        }
        else
        {
            *ip -= 1;
            return &cpu->RAM[arg];
        }
    }
    
    else if (command & ARG_REG) 
    {
        memcpy(&curReg, cpu->cmdArr + *ip, sizeof(int));
        *ip += sizeof(int) - 1;

        return cpu->regs + curReg;
    }

    else
    {
        return NULL;
    }

    return NULL;
}

int GetJumpArg(int* ip, Cpu_t* cpu)
{
    int arg = 0;
    
    memcpy(&arg, cpu->cmdArr + *ip, sizeof(int));
    if (arg < 0)
    {
        return 1;
    }

    *ip = (int) arg - 1;

    return 0;
}

void CpuDump(Cpu_t* cpu, int ip)
{
    printf("-----------------------------------CPU DUMP-----------------------------------\n");

    
    for (int index = 0; index < cpu->size; ++index)
    {   
        if (index == ip)
        {
            printf("[");
        }
        
        printf("(%02X, %02X) ", index, *(cpu->cmdArr + index));

        if (index == ip)
        {
            printf("]");
        }


        if (*(cpu->cmdArr + index) == CMD_HLT) 
            break;

        if (index != 0 && index % 9 == 0)
            printf("\n");
    }

    printf("\n");
#if DEBUG_MODE
    StackDump(&cpu->stk);
#endif

    printf("                            REGISTERS:\n");
    printf("REG:  VALUE:\n");

    for (int index = 1; index < REGS_COUNT; ++index)
    {
        printf("R%cX %lf\n", 'A' + index - 1, cpu->regs[index]);
    }

    printf("--------------------Function call stack--------------------\n");
#if DEBUG_MODE
    StackDump(&cpu->retStk);
#endif

    printf("                            RAM:\n");

    for (int index = 0; index < MAX_RAM_SIZE; ++index)
    {
        printf("%3lg ", cpu->RAM[index]);

        if ((index + 1) % 10 == 0)
            printf("\n");
    }

    printf("--------------------------------------------------------------------------------\n");
}

int DoubleCmp(double num1, double num2)
{
	return (fabs(num1 - num2) > EPS) ? 1 : 0;
}
