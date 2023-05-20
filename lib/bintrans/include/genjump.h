DEF_JMP(JMP, 1, "\xe9")                    // jmp

DEF_JMP(CALL, 1, "\xe8")                   // call

DEF_JMP(JE,  7, "\x5e\x5f\x48\x39\xf7\x0f\x84") // 0: pop rsi; 
                                                // 1: pop rdi; 
                                                // 2: cmp rsi, rdi; 
                                                // 5: je

DEF_JMP(JNE, 7, "\x5e\x5f\x48\x39\xf7\x0f\x85")

DEF_JMP(JG,  7, "\x5e\x5f\x48\x39\xf7\x0f\x8f")

DEF_JMP(JGE, 7, "\x5e\x5f\x48\x39\xf7\x0f\x8d")

DEF_JMP(JL,  7, "\x5e\x5f\x48\x39\xf7\x0f\x8c")

DEF_JMP(JLE, 7, "\x5e\x5f\x48\x39\xf7\x0f\x8e")


