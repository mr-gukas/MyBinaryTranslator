DEF_PUSH(IMM, 3, "\x48\xc7\xc7",                                        // mov rdi, IMM 
{
    binCodeMemcpy(code, code->data_size, &imm_val, sizeof(int));
    binCodeAppend(code, "\x57", 1);                                     // push rdi
})

DEF_PUSH(REG, 0, "", 
{
    reg_val += 0x50;                                                    // push r_x
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));
})

DEF_PUSH(IMM_MEM, 4, "\x48\x8b\x3c\x25",                               // push rdi, qword IMM
{
    binCodeMemcpy(code, code->data_size, &imm_val, sizeof(int));
    binCodeAppend(code, "\x57", 1);                                    // push rdi
})

DEF_PUSH(REG_MEM, 2, "\x48\x8b",                                       // mov rdi, qword [r_x]  
{
    reg_val += 0x38;
         
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));
    binCodeAppend(code, "\x57", 1);                                    // push rdi
})

DEF_PUSH(IMM_REG_MEM, 2, "\x48\x8b",                                  // mov rdi, qword [r_x + IMM]  
{
    reg_val += 0xb8;
         
    binCodeMemcpy(code, code->data_size, &reg_val, sizeof(char));
    binCodeMemcpy(code, code->data_size, &imm_val, sizeof(int));
    binCodeAppend(code, "\x57", 1);                                   // push rdi
})
