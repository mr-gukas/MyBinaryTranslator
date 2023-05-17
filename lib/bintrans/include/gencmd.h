//---------------------------------------------------------
DEF_OPER(Add,  24, "\xf2\x0f\x10\x4c\x24\x08\xf2\x0f\x10\x14\x24\x48\x83\xc4\x08\xf2\x0f\x58\xca\xf2\x0f\x11\x0c\x24")

DEF_OPER(Sub,  24, "\xf2\x0f\x10\x4c\x24\x08\xf2\x0f\x10\x14\x24\x48\x83\xc4\x08\xf2\x0f\x5c\xca\xf2\x0f\x11\x0c\x24")

DEF_OPER(Mul,  24, "\xf2\x0f\x10\x4c\x24\x08\xf2\x0f\x10\x14\x24\x48\x83\xc4\x08\xf2\x0f\x59\xca\xf2\x0f\x11\x0c\x24")

DEF_OPER(Div,  24, "\xf2\x0f\x10\x4c\x24\x08\xf2\x0f\x10\x14\x24\x48\x83\xc4\x08\xf2\x0f\x5e\xca\xf2\x0f\x11\x0c\x24")

DEF_OPER(Pow,  14, "\xf2\x0f\x10\x04\x24\x66\x0f\x51\xc0\xf2\x0f\x11\x04\x24")

DEF_OPER(IsBt,  5, "\x5e\x5f\x48\x39\xf7")

//---------------------------------------------------------
DEF_POP(IMM_MEM, 4, "\x5f\x48\x89\x3d", 
{
    char pop_arg[4] = "";
    memcpy(pop_arg, &imm_val, 4);
    binCodeAppend(code, pop_arg, 4);
})

DEF_POP(REG_MEM, 3, "\x5f\x48\x89",
{
    char pop_arg[1] = "";
    reg_val += 0x38;
    memcpy(pop_arg, &reg_val, 1);

    binCodeAppend(code, pop_arg, 1);
})

DEF_POP(REG, 0, "", 
{
    char pop_arg[1] = "";
    reg_val += 0x58;
    memcpy(pop_arg, &reg_val, 1);

    binCodeAppend(code, pop_arg, 1);
})

DEF_POP(IMM_REG_MEM, 3, "\x5f\x48\x89", 
{
    char pop_arg[5] = ""; 
    reg_val += 0x78;
    memcpy(pop_arg, &reg_val, 1);
    memcpy(pop_arg + 1, &imm_val, 4);

    binCodeAppend(code, pop_arg, 5);
})

//---------------------------------------------------------
DEF_PUSH(IMM, 3, "\x48\xc7\xc7", 
{
    char push_arg[4] = "";
    memcpy(push_arg, &imm_val, 4);

    binCodeAppend(code, push_arg, 4);
    binCodeAppend(code, "\x57", 1);
})

DEF_PUSH(REG, 0, "", 
{
    char push_arg[1] = "";
    reg_val += 0x50;
    memcpy(push_arg, &reg_val, 1);
         
    binCodeAppend(code, push_arg, 1);
})

DEF_PUSH(IMM_MEM, 4, "\x48\x8b\x3c\x25", 
{
    char push_arg[4] = "";
    memcpy(push_arg, &imm_val, 4);
         
    binCodeAppend(code, push_arg, 4);
    binCodeAppend(code, "\x57", 1);

})

DEF_PUSH(REG_MEM, 2, "\x48\x8b", 
{
    char push_arg[1] = "";
    reg_val += 0x38;
    memcpy(push_arg, &reg_val, 1);
         
    binCodeAppend(code, push_arg, 1);
    binCodeAppend(code, "\x57", 1);
})

DEF_PUSH(IMM_REG_MEM, 2, "\x48\x8b", 
{
    char push_arg[5] = "";
    reg_val += 0xb8;
    memcpy(push_arg, &reg_val, 1);
    memcpy(push_arg + 1, &imm_val, 4);
         
    binCodeAppend(code, push_arg, 5);
    binCodeAppend(code, "\x57", 1);
})

DEF_JMP(JMP, 1, "\xe9")

DEF_JMP(CALL, 1, "\xe8")

DEF_JMP(JE,  7, "\x5e\x5f\x48\x39\xf7\x0f\x84")

DEF_JMP(JNE, 7, "\x5e\x5f\x48\x39\xf7\x0f\x85")

DEF_JMP(JG,  7, "\x5e\x5f\x48\x39\xf7\x0f\x8f")

DEF_JMP(JGE, 7, "\x5e\x5f\x48\x39\xf7\x0f\x8d")

DEF_JMP(JL,  7, "\x5e\x5f\x48\x39\xf7\x0f\x8c")

DEF_JMP(JLE, 7, "\x5e\x5f\x48\x39\xf7\x0f\x8e")




