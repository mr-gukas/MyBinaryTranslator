DEF_POP(IMM_MEM, 4, "\x5f\x48\x89\x3d", 
{
    binCodeMemcpy(code, code->data_size, &imm_val, sizeof(int));
})

DEF_POP(REG_MEM, 3, "\x5f\x48\x89",
{
    reg_val += 0x38;
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));
})

DEF_POP(REG, 0, "", 
{

    reg_val += 0x58;
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));
})

DEF_POP(IMM_REG_MEM, 3, "\x5f\x48\x89", 
{
    reg_val += 0x78;
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));
    binCodeMemcpy(code, code->data_size, &imm_val, sizeof(int));
})

