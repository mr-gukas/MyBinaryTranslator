DEF_POP(IMM_MEM, 4, "\x5f\x48\x89\x3d",                              // pop rdi
{
    binCodeMemcpy(code, code->data_size, &imm_val, sizeof(int));     // mov [IMM], rdi
})

DEF_POP(REG_MEM, 3, "\x5f\x48\x89",                                    // pop rdi
{
    reg_val += 0x38;
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));     // mov [r_x], rdi
})

DEF_POP(REG, 0, "",                                                    // pop r_x 
{
    reg_val += 0x58; 
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));
})

DEF_POP(IMM_REG_MEM, 3, "\x5f\x48\x89",                               // pop rdi
{
    reg_val += 0xb8;
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));    // mov [r_x + IMM], rdi
    binCodeMemcpy(code, code->data_size, &imm_val, sizeof(int));
})

