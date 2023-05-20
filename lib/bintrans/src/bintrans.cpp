#include "../include/bintrans.hpp"
    
int main(void)
{
//-----------------------------------------------
// load AST from file
	Tree_t  stTree = {};
    readAST(&stTree, AST_FILE_NAME);
//-----------------------------------------------
// from AST to binary code  
	Program_t* program = WriteBinCode(stTree.root);
//-----------------------------------------------
// generate elf file 
    gen_elf(program->code->data, program->code->size);
//-----------------------------------------------
// end of process
#if DEBUG_MODE 
    endLog();
#endif
    ProgramDtor(program);
    TreeDtor(&stTree);
    free(program);
//-----------------------------------------------
	return 0;
}

Program_t* WriteBinCode(TreeNode_t* root)
{
	assert(root); 

	Program_t*  program = (Program_t*) calloc(1, sizeof(Program_t));
	ProgramCtor(program, root);

	ParseStatement(program->node, program);
    
    #if DEBUG_MODE
        fprintf(log_file, "FULL DUMP:\n");
        hexDump(program->code->data, program->code->data_size, log_file);
    #endif
    
    return program;
}

int ProgramCtor(Program_t* program, TreeNode_t* node) {
	if (program == NULL) return WRONG_DATA;
	if (node    == NULL) return NULL_TREE;
	
	program->node    = node;
	program->funcArr = FuncArrCtor();
    program->code    = binCodeCtor();
	program->tabStk  = (Stack_t*) calloc(1, sizeof(Stack_t));	

	StackCtor(program->tabStk, 10);

	return OK;
}

int ProgramDtor(Program_t* program) {
    if (program == NULL) return WRONG_DATA;

    for (size_t index = 0; index < program->tabStk->size; index++) {
		VarTableDtor(program->tabStk->data[index]);
	}

	StackDtor  (program->tabStk);	
	FuncArrDtor(program->funcArr);
    binCodeDtor(program->code);
	free       (program->tabStk);

    return OK;
}

void ParseStatement(TreeNode_t* node, Program_t* program)
{
	assert(node && program);
	assert(node->type == Type_STAT);
		
	VarTable_t* oldTable = NULL;

	if (program->tabStk->size != 0)
		oldTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

	if (program->tabStk->size != 0 && (node->parent && node->parent->type != Type_FUNC))
	{
        genPush(program->code, PUSH_REG, 0,                RAX_MASK); 
        genPush(program->code, PUSH_IMM, QWORD * oldTable->size,  0);
        genOper(program->code, Op_Add);
        genPop (program->code, POP_REG,  0,                RAX_MASK);
	}

	VarTable_t* newTable = NULL;
	
	if (!node->parent || (node->parent && node->parent->type != Type_FUNC))
	{
		newTable = VarTableCtor(program->tabStk);
	}
	
	TreeNode_t* curNode = node;
	int firstFunc       = 0;	
	int isRet           = 1;
	while (curNode)
	{
		if (curNode->left->type == Type_FUNC && !firstFunc) 
		{
            binCodeAppend(program->code, SET_RAM, SET_RAM_SIZE);
            
            uint64_t addr = ENTRY_POINT + PAGE_SIZE; 
            binCodeMemcpy(program->code, program->code->data_size, &(addr), 8);
            
            uint64_t scanf_buf = ENTRY_POINT + PAGE_SIZE + MEM_SZ + 
                                 SCANF_SIZE; 
            
            binCodeAppend(program->code, SET_SCANF_BUF, SET_SCANF_BUF_SIZE);
            binCodeMemcpy(program->code, program->code->data_size, &(scanf_buf), 8);


            uint64_t printf_buf = ENTRY_POINT + PAGE_SIZE + MEM_SZ + 
                                  SCANF_SIZE  + SCANF_BUF + PRINTF_SIZE; 
            
            binCodeAppend(program->code, SET_PRINTF_BUF, SET_PRINTF_BUF_SIZE);
            binCodeMemcpy(program->code, program->code->data_size, &(printf_buf), 8);

            uint64_t itoa_buf = printf_buf + PRINTF_BUF;
            binCodeAppend(program->code, SET_ITOA_BUF, SET_ITOA_BUF_SIZE);
            binCodeMemcpy(program->code, program->code->data_size, &(itoa_buf), 8);

            genJump(program->code, JMP_JMP, 0);
			firstFunc = 1;
		}

		switch(curNode->left->type)
		{
			case Type_DEF:    ParseDef    (curNode->left, program); break;
			case Type_ASSIGN: ParseAssign (curNode->left, program); break;
			case Type_FUNC:	  ParseFunc   (curNode->left, program); break;
			case Type_PRINTF: ParsePrintf (curNode->left->left, program); break;
			case Type_SCANF:  ParseScanf  (curNode->left->left, program); break;
			case Type_CALL:   ParseCall   (curNode->left, program); break;
			case Type_IF:     ParseIf     (curNode->left, program); break;
			case Type_WHILE:  ParseWhile  (curNode->left, program); break;
			case Type_RETURN: isRet = 1; ParseRet (curNode->left, program); break;

			default: abort(); 
		}
		
		curNode = curNode->right;
	}

	if (!isRet && program->tabStk->size != 0 && (node->parent && node->parent->type != Type_FUNC))
	{
        genPush(program->code, PUSH_REG, 0,                RAX_MASK); 
        genPush(program->code, PUSH_IMM, QWORD * oldTable->size, 0);
        genOper(program->code, Op_Sub);
        genPop (program->code, POP_REG,  0,                RAX_MASK);
	}

	
}

int ParseFunc(TreeNode_t* node, Program_t* program)
{
	for (size_t index = 0; index < program->funcCount; index++)
	{
		if (STR_EQ((program->funcArr[index]).name, node->left->name))
		{
			return ALREADY_DEF_FUNC;
		}
	}
    
    if (STR_EQ("main", node->left->name)) {
        regenJump(program->code, 41);
        program->inMain = 1;
    }

	(program->funcArr[program->funcCount]).name = node->left->name;

	if (STR_EQ(node->left->right->name, "krknaki"))
		(program->funcArr[program->funcCount]).retVal = INT;
	else
		(program->funcArr[program->funcCount]).retVal = VOID;
	
	VarTable_t* newTable = VarTableCtor(program->tabStk);
	
	if (node->left->left)
		(program->funcArr[program->funcCount]).parCount = ParseParams(node->left->left, newTable);
	
    program->funcArr[program->funcCount].func_start = program->code->data_size;
	program->funcCount++;
	ParseStatement(node->right, program);

	VarTableDtor(newTable);
	StackPop(program->tabStk);			

    program->inMain = 0;
	
	return OK;
}

int VarTableDtor(VarTable_t* table)
{
	free(table->arr);
	free(table);

	return OK;
}

Func_t* FuncArrCtor()
{
	Func_t* funcArr = (Func_t*) calloc(FUNC_COUNT, sizeof(Func_t));
	
	return funcArr; 
}

int FuncArrDtor(Func_t* funcArr)
{
	free(funcArr);

	return OK;
}

int ParseCall(TreeNode_t* node, Program_t* program)
{
	int isDefFunc    = 0;
	size_t funcParam = 0;
	size_t callParam = 0;
	
    size_t func_index = 0;
	for (func_index = 0; func_index < program->funcCount; func_index++)
	{
		if (STR_EQ((program->funcArr[func_index]).name, node->left->name))
		{
			isDefFunc = 1; 
			funcParam = (program->funcArr[func_index]).parCount;

			break;	
		}
	}
	
	if (!isDefFunc)
	{
		UNDEF_FUNC(node->left);
	}

	
	if (node->left->left)
		callParam = CountParams(node->left->left);
	
	if (funcParam != callParam)
	{
		return BAD_CALL;
	}
	
	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

    genPush(program->code, PUSH_REG, 0,                RAX_MASK); 
    genPush(program->code, PUSH_IMM, QWORD * curTable->size, 0);
    genOper(program->code, Op_Add);
    genPop (program->code, POP_REG,  0,                RAX_MASK);

	for (int index = (int) callParam - 1; index >= 0; index--)
	{
        genPop(program->code, POP_IMM_REG_MEM, QWORD * index, RAX_MASK);
	}
    
    genJump(program->code, JMP_CALL, -5 - program->code->data_size + program->funcArr[func_index].func_start);
    genPush(program->code, PUSH_REG, 0,                RAX_MASK); 
    genPush(program->code, PUSH_IMM, QWORD * curTable->size, 0);
    genOper(program->code, Op_Sub);
    genPop (program->code, POP_REG,  0,                RAX_MASK);

	return OK;
}

size_t CountParams(TreeNode_t* node)
{
	TreeNode_t* curNode  = node;
	size_t      parCount = 0;

	while (curNode)
	{
		parCount++;
		curNode = curNode->right;
	}

	return parCount;
}

VarTable_t* VarTableCtor(Stack_t* stk)
{
	VarTable_t* newTable = (VarTable_t*) calloc(1, sizeof(VarTable_t));

	if (stk->size >= 1)
	{
		VarTable_t* curTable = 	GetTableFromStk(stk, stk->size - 1);
		newTable->raxPos     = curTable->raxPos + curTable->size; 
	}
	else
		newTable->raxPos = 0;
	
	newTable->arr = (Variable_t*) calloc(VAR_COUNT, sizeof(Variable_t));
	StackPush(stk, newTable);

	return newTable;
}

size_t ParseParams(TreeNode_t* node, VarTable_t* table)
{
	TreeNode_t* curNode = node;
	size_t parCount = 0;

	while (curNode)
	{
		parCount++;

		Variable_t param        = {};
		param.name              = curNode->left->name;
		param.pos               = table->size;
		table->arr[table->size] = param;

		table->size++;
		curNode = curNode->right;
	}

	return parCount;
}

void Printf4Translator(int num) {
#if TEST_MODE
    printf("%d\n", num);
#else
    printf("Output: %d\n", num);
#endif
}

int ParsePrintf(TreeNode_t* node, Program_t* program) 
{
	TreeNode_t* curNode = node;
	while (curNode)
	{
		if (!CheckForVar(node->left, program->tabStk))
		{
			UNDEF_VAR(node->left);
		}
	    	
        #if DEBUG_MODE    
            fprintf(log_file, "OUTPUT\n");
        #endif

		int varPos = GetVarPos(node->left, program->tabStk);

        genPush(program->code, PUSH_IMM_REG_MEM, QWORD * varPos, RAX_MASK);

        binCodeAppend(program->code, MOV_RBP_RSP,    MOV_RBP_RSP_SIZE);
        binCodeAppend(program->code, PUSH_REGS,      PUSH_REGS_SIZE);
        binCodeAppend(program->code, AND_RSP_FF,     AND_RSP_FF_SIZE);
        binCodeAppend(program->code, MOVABS_R10_IMM, MOVABS_R10_IMM_SIZE);

        //uint64_t addr = (uint64_t) ((void *) Printf4Translator); 

        uint64_t addr = ENTRY_POINT + PAGE_SIZE + MEM_SZ + SCANF_SIZE + SCANF_BUF;  
        binCodeMemcpy(program->code, program->code->data_size, &addr, 8);

        binCodeAppend(program->code, CALL_R10,    CALL_R10_SIZE);
        binCodeAppend(program->code, POP_REGS,    POP_REGS_SIZE);
        binCodeAppend(program->code, MOV_RSP_RBP, MOV_RSP_RBP_SIZE);
        binCodeAppend(program->code, POP_RDI,     POP_RDI_SIZE);

		curNode = curNode->right;
	}
	
	return OK;
}



int ParseScanf(TreeNode_t* node, Program_t* program)
{
	TreeNode_t* curNode = node;
	while (curNode)
	{
		if (!CheckForVar(node->left, program->tabStk))
		{
			UNDEF_VAR(node->left);
		}
		
		int varPos = GetVarPos(node->left, program->tabStk);

        #if DEBUG_MODE
            fprintf(log_file, "INPUT\n");
        #endif 

        binCodeAppend(program->code, PUSH_RDI,       PUSH_RDI_SIZE);
        binCodeAppend(program->code, LEA_RDI_RSP,    LEA_RDI_RSP_SIZE);
        binCodeAppend(program->code, PUSH_REGS,      PUSH_REGS_SIZE);
        binCodeAppend(program->code, MOV_RBP_RSP,    MOV_RBP_RSP_SIZE);
        binCodeAppend(program->code, AND_RSP_FF,     AND_RSP_FF_SIZE);
        binCodeAppend(program->code, MOVABS_R10_IMM, MOVABS_R10_IMM_SIZE);
        

        //uint64_t addr = (uint64_t) ((void *) Scanf4Translator);  

        uint64_t addr = ENTRY_POINT + PAGE_SIZE + MEM_SZ;  
        binCodeMemcpy(program->code, program->code->data_size, &addr, 8);

        binCodeAppend(program->code, CALL_R10,    CALL_R10_SIZE);
        binCodeAppend(program->code, MOV_RSP_RBP, MOV_RSP_RBP_SIZE);
        binCodeAppend(program->code, POP_REGS,    POP_REGS_SIZE);
         
        genPop(program->code, POP_IMM_REG_MEM, varPos, RAX_MASK);

		curNode = curNode->right;
	}
	
	return OK;
}	



int ParseDef(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->right, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}

	if (CheckForVar(node->left, program->tabStk))
	{
		ALREADY_DEF_VAR(node->left);
	}
	
	Variable_t newVar    = {}; 
	VarTable_t* curTable = GetTableFromStk(program->tabStk,  program->tabStk->size - 1);

	newVar.name = node->left->name;
	newVar.pos  = curTable->size;
		
	curTable->arr[curTable->size] = newVar;
	curTable->size               += 1;
		
	CountExpression(node->right, program);
    
    genPop(program->code, POP_IMM_REG_MEM, QWORD * newVar.pos, RAX_MASK);

	return OK;
}

int ParseAssign(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->right, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}
	
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->left);
	}
	
	int pos = GetVarPos(node->left, program->tabStk);
	CountExpression(node->right, program);
	
    genPop(program->code, POP_IMM_REG_MEM, QWORD * pos, RAX_MASK);
	
	return OK;
}

int ParseIf(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}
	
	CountExpression(node->left, program);

    genPush(program->code, PUSH_IMM, 0, 0);
	
	if (node->right->type != Type_ELSE)
	{
        size_t jmp_addr = program->code->data_size + 7;
        genJump(program->code, JMP_JE, 0);

		ParseStatement(node->right, program);

		VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		VarTableDtor(curTable);
		StackPop(program->tabStk);			
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

        genPush(program->code, PUSH_IMM, curTable->raxPos,          0);
        genPop (program->code, POP_REG,                 0, RAX_MASK);

        regenJump(program->code, jmp_addr);       
	}
	else
	{
        size_t else_addr = program->code->data_size + 7;
        genJump(program->code, JMP_JE, 0);
	
		ParseStatement(node->right->left, program);
		VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		VarTableDtor(curTable);
		StackPop(program->tabStk);			
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

        genPush(program->code, PUSH_IMM, curTable->raxPos,          0);
        genPop (program->code, POP_REG,                 0, RAX_MASK);
        
        size_t if_addr = program->code->data_size + 7; 
        genJump(program->code, JMP_JMP, 0);

        regenJump(program->code, else_addr);

		ParseStatement(node->right->right, program);
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		VarTableDtor(curTable);
		StackPop(program->tabStk);			
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

        genPush(program->code, PUSH_IMM, curTable->raxPos,          0);
        genPop (program->code, POP_REG,                 0, RAX_MASK);
        
	    regenJump(program->code, if_addr - 6);	
	}
	
	return OK;
}

int ParseWhile(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}
	
    size_t while_addr = program->code->data_size;
	CountExpression(node->left, program);

    genPush(program->code, PUSH_IMM, 0, 0);

    size_t while_no_addr = program->code->data_size + 7;
    genJump(program->code, JMP_JE, 0);

	ParseStatement(node->right, program);
	
	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
	VarTableDtor(curTable);
	StackPop(program->tabStk);			
	
	curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

    genPush(program->code, PUSH_IMM, curTable->raxPos,          0);
    genPop (program->code, POP_REG,                 0, RAX_MASK);
 
    genJump(program->code, JMP_JMP, program->code->data_size - while_addr);
    regenJump(program->code, while_no_addr);
	
	return OK;
}

int ParseRet(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}	

	int isInLocal = 0;
	TreeNode_t* curNode = node;
	while (curNode->type != Type_FUNC)
	{
		if (curNode->type == Type_IF    ||
			curNode->type == Type_ELSE  ||
			curNode->type == Type_WHILE)
		{
			isInLocal = 1;
		}

		curNode = curNode->parent;
	}
    
    if (!program->inMain){
	    CountExpression(node->left,  program);
    }
    else {
        binCodeAppend(program->code, SET_EXIT, SET_EXIT_SIZE);
    }

	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 2);
	if (isInLocal)
	{
        genPop (program->code, POP_REG,                 0, RCX_MASK);
        genPush(program->code, PUSH_REG,                0, RAX_MASK); 
        genPush(program->code, PUSH_IMM, QWORD * curTable->size,          0);
        genOper(program->code, Op_Sub);
        genPop (program->code, POP_REG,                 0, RAX_MASK);

	}
    
    genRet(program->code);

	return OK;
}

void CountExpression(TreeNode_t* node, Program_t* program)
{
	if (node->left)  CountExpression(node->left, program);
	if (node->right) CountExpression(node->right, program);
	
	if (node->type == Type_NUM)
	{
        genPush(program->code, PUSH_IMM, node->numVal, 0);
	}
	else if (node->type == Type_VAR)
	{
		int varPos = GetVarPos(node, program->tabStk);
        genPush(program->code, PUSH_IMM_REG_MEM, QWORD * varPos, RAX_MASK);
	}
	else if (node->type == Type_CALL)
	{
		ParseCall(node, program);
        genPush(program->code, PUSH_REG, 0, RCX_MASK);
	}

#if DEBUG_MODE
    #define DEF_OPER(op_type, size, bin_code)                  \
            case Op_##op_type:                                 \
                binCodeAppend(code, bin_code, size);           \
                fprintf(log_file, "OP: %s\n", bin_code);       \
                break;
#else
    #define DEF_OPER(op_type, size, bin_code)              \
        case Op_##op_type:                                 \
            binCodeAppend(code, bin_code, size);           \
            break;
#endif

	if (node->type == Type_OP)
	{
        genOper(program->code, node->opVal);
	}

    #undef DEF_OPER
}


int CheckForVar(TreeNode_t* node, Stack_t* stk)
{
	if (node->left)
	{
		if (!CheckForVar(node->left, stk)) return 0;
	}
	
	if (node->right)
	{
		if (!CheckForVar(node->right, stk)) return 0;
	}

	size_t stkIndex      =  stk->size - 1;
	VarTable_t* curTable = GetTableFromStk(stk, stkIndex);
	
	if (node->type == Type_VAR)
	{
		while (stkIndex >= 0)
		{
			for (size_t varIndex = 0; varIndex < curTable->size; varIndex++)
			{
				if (STR_EQ(node->name, (curTable->arr[varIndex]).name))
				{
					return 1; //if we met the variable
				}
			}
			
			if (stkIndex == 0)
				break;
	
			stkIndex--;
			curTable = GetTableFromStk(stk, stkIndex);
		}

		return 0; //if variable is not defined
	}
		
	return 1;
}
				
VarTable_t* GetTableFromStk(Stack_t* stk, size_t index)
{
	return stk->data[index];
}


int GetVarPos(TreeNode_t* node, Stack_t* stk)
{
	size_t stkIndex      = stk->size - 1;
	VarTable_t* curTable = GetTableFromStk(stk, stkIndex);
	size_t       lastRax = curTable->raxPos;

	while (stkIndex >= 0)
	{
		for (size_t varIndex = 0; varIndex < curTable->size; varIndex++)
		{
			if (STR_EQ(node->name, (curTable->arr[varIndex]).name))
			{
				return (int)(curTable->arr[varIndex]).pos + (int)curTable->raxPos - (int)lastRax;
			}
		}
		
		if (stkIndex == 0)
		{
			break;
		}

		stkIndex--;
		curTable = GetTableFromStk(stk, stkIndex);
	}
	return POISON_PTR;
}

int readAST(Tree_t* tree, const char* ast_file) {
    if (tree == NULL || ast_file == NULL) return 1;

	FILE* standart = fopen(ast_file, "r");
	assert(standart);

	tree->root = ReadStandart(standart);

	fclose(standart);

	TreeUpdate(tree, tree->root);
	TreeDump(tree);
    
    return 0;
}

bin_code_t* binCodeCtor() {
    bin_code_t* bin_code = (bin_code_t*) calloc(1, sizeof(bin_code_t));
    if (!bin_code) {
        fprintf(stderr, "can't allocate memory for bincode...\n");
        return NULL;
    }

    bin_code->data = (char*) aligned_alloc(4096, 4096);
    memset(bin_code->data, 0xc3, 4096);

    if (!bin_code) {
        fprintf(stderr, "can't allocate memory for bincode...\n");
        return NULL;
    }

    bin_code->size      = 4096;
	bin_code->data_size = 0;

	return bin_code;
}

int binCodeDtor(bin_code_t* bin_code) {
    if (bin_code == NULL) return 1;

    free(bin_code->data);
    free(bin_code);

    return 0;
}

int binCodeAppend(bin_code_t* bin_code, const char* data, size_t data_size) {
    if (bin_code == nullptr || data == nullptr || data_size == 0) return 1;
    
    if (bin_code->size - bin_code->data_size < data_size) {
        while (bin_code->size - bin_code->data_size < data_size) {
            bin_code->size *= 2;
        }
        
        bin_code->data = (char*) realloc(bin_code->data, bin_code->size);
    }
    
    memcpy(bin_code->data + bin_code->data_size, data, data_size);
    bin_code->data_size += data_size; 

#if DEBUG_MODE
    fprintf(log_file, "MEMCPY HEXDUMP:\n");
    hexDump(data, data_size, log_file);
    fprintf(log_file, "code size: 0x%lx\n", bin_code->data_size);

#endif

    return 0;
}

static void genRet(bin_code_t* code) {
    binCodeAppend(code, "\xc3", 1);

#if DEBUG_MODE
    fprintf(log_file, "RET\n");
#endif
}


static void genPop(bin_code_t* code,  PopType type, int imm_val,  char reg_val) {
#if DEBUG_MODE
    #define DEF_POP(type, size, bin_code, action)                                      \
            case POP_##type:                                                           \
            {                                                                          \
                fprintf(log_file, "POP_"#type "\n\tIMM: %d\n\tREG: %s\n", imm_val, reg_name[reg_val]); \
                binCodeAppend(code, bin_code, size);                                   \
                action;                                                                \
            break;                                                                     \
            }
#else
    #define DEF_POP(type, size, bin_code, action)                                      \
            case POP_##type:                                                           \
            {                                                                          \
                fprintf(log_file, "POP_"#type "\n\tIMM: %d\n\tREG: %s\n", imm_val, reg_name[reg_val]); \
                binCodeAppend(code, bin_code, size);                                   \
                action;                                                                \
            break;                                                                     \
            }
#endif

    switch(type)
    {
        #include "../include/genpop.h"
		default: break;
    }
    
    #undef DEF_POP
}

static void genPush(bin_code_t* code,  PushType type, int imm_val,  char reg_val) {
#if DEBUG_MODE
    #define DEF_PUSH(type, size, bin_code, action)                                                 \
        case PUSH_##type:                                                                          \
        {                                                                                          \
            fprintf(log_file, "PUSH_"#type"\n\tIMM: %d\n\tREG: %s\n", imm_val, reg_name[reg_val]); \
            binCodeAppend(code, bin_code, size);                                                   \
            action;                                                                                \
                break;                                                                             \
        }
#else
    #define DEF_PUSH(type, size, bin_code, action)                                                 \
        case PUSH_##type:                                                                          \
        {                                                                                          \
            binCodeAppend(code, bin_code, size);                                                   \
            action;                                                                                \
                break;                                                                             \
        }
#endif


    switch(type)
    {
        #include "../include/genpush.h"
		default: break;
    }
   
    #undef DEF_PUSH
}

static void genOper(bin_code_t* code, OperationType type) {
#if DEBUG_MODE
    #define DEF_OPER(op_type, size, bin_code)       \
        case Op_##op_type:                          \
        {                                           \
            fprintf(log_file, #op_type "\n");       \
            binCodeAppend(code, bin_code, size);    \
            break;                                  \
        }
#else
    #define DEF_OPER(op_type, size, bin_code)       \
        case Op_##op_type:                          \
        {                                           \
            binCodeAppend(code, bin_code, size);    \
            break;                                  \
        }
#endif

    switch (type)
	{
        #include "../include/genoper.h"
		default: break;
    }

    #undef DEF_OPER
}

static void genJump(bin_code_t* code, JumpType type, int shift) {
#if DEBUG_MODE
    #define DEF_JMP(jmp_type, size, bin_code)                   \
        case JMP_##jmp_type:                                    \
        {                                                       \
            binCodeAppend(code, bin_code, size);                \
            fprintf(log_file, "JMP_"#jmp_type " 0x%x ", shift); \
            break;                                              \
        }
#else
    #define DEF_JMP(jmp_type, size, bin_code)                   \
        case JMP_##jmp_type:                                    \
        {                                                       \
            binCodeAppend(code, bin_code, size);                \
            fprintf(log_file, "JMP_"#jmp_type " 0x%x ", shift); \
            break;                                              \
        }
#endif

    switch(type)
    {
        #include "../include/genjump.h"    
        default:  break;
    }

    #undef DEF_JMP
    
    binCodeMemcpy(code, code->data_size, &shift, sizeof(int));
}

static void regenJump(bin_code_t* code, int arg) {
    int shift = code->data_size - arg - 4; 
    binCodeMemcpy(code, arg, &shift, sizeof(int));

#if DEBUG_MODE
    fprintf(log_file, "REGEN JUMP ARG in 0x%x on 0x%x\n", arg, shift);
#endif
}

static void binCodeMemcpy(bin_code_t* code, size_t dest, void* arg, size_t size) {
    assert(code);

    if (dest == code->data_size) {
        if (code->size - code->data_size < size) {
            while (code->size - code->data_size < size) {
                code->size *= 2;
            }
            
            code->data = (char*) realloc(code->data, code->size);
        }
        
        code->data_size += size; 
    }

    memcpy(code->data + dest, arg, size);

#if DEBUG_MODE
    fprintf(log_file, "MEMCPY HEXDUMP:\n");
    hexDump(arg, size, log_file);
    fprintf(log_file, "code size: 0x%lx\n", code->data_size);
#endif
}

void hexDump (const void * addr, const int len, FILE* file) {
    assert(addr && len && file);

    int index                     = 0;
    unsigned char buff[PERLINE+1] = {};
    const unsigned char* pc       = (const unsigned char*) addr;

    for (index = 0; index < len; index++) {

        if ((index % PERLINE) == 0) {
            if (index != 0) 
                fprintf(file, "  %s\n", buff);
        }

        fprintf(file, " %02x", pc[index]);
    }

    while ((index % PERLINE) != 0) {
        fprintf(file, "   ");
        index++;
    }
    fprintf(file, "  %s\n", buff);
}

static void binCodeRun(bin_code_t* code) {
    assert(code);
   
    int res = mprotect (code->data, code->data_size, PROT_EXEC | PROT_WRITE | PROT_READ);
    if (res != 0) { printf ("Your memory is not aligned by 4096!\n"); return; }
    
    void (* JIT) (void) = (void (*) (void)) (code->data);

    SAVEREGS
        JIT();
    RESTOREREGS
}

void Scanf4Translator(int* num) {
#ifndef TEST_MODE
    printf("Input: ");
#endif

    scanf("%d", num);
}



