#include "bintrans.hpp"
#include <cstdlib>
    
int main(void)
{
//-----------------------------------------------
// load AST from file
	Tree_t  stTree = {};
    readAST(&stTree, AST_FILE_NAME);
//-----------------------------------------------
// from AST to binary code  
	WriteBinCode(stTree.root);
//-----------------------------------------------
// for debugging
    #ifdef DEBUG_MODE
        endLog();
    #endif

	return 0;
}

int WriteAsmCode(TreeNode_t* root)
{
	assert(root); 

	Program_t   program = {};
	ProgramCtor(&program, root);
	
	ParseStatement(program.node, &program);

    ProgramDtor(&program);
	return 0;
}

int ProgramCtor(Program_t* program, TreeNode_t* node) {
	if (program == NULL) return WRONG_DATA;
	if (node    == NULL) return NULL_TREE;
	
	program->node   = node;
	program->tabStk = (Stack_t*) calloc(1, sizeof(Stack_t));	
	StackCtor(program->tabStk, 10);
	program->funcArr = FuncArrCtor();
    program->code    = binCodeCtor(64);
    program->ram_arr = (char*) calloc(224, sizeof(char));
    program->pass    = 1;

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
    free       (program->ram_arr);
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
        genPush(program->code, PUSH_REG, 0,                RCX_LETTER); 
        genPush(program->code, PUSH_IMM, 4*oldTable->size, 0);
        genOper(program->code, Op_Add);
        genPop (program->code, POP_REG,  0,                RCX_LETTER);

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
		if (curNode->left->type == Type_FUNC && !firstFunc) //TODO: genJmp on main
		{
            //genJmp(main)
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
        genPush(program->code, PUSH_REG, 0,                RCX_LETTER); 
        genPush(program->code, PUSH_IMM, 4*oldTable->size, 0);
        genOper(program->code, Op_Sub);
        genPop (program->code, POP_REG,  0,                RCX_LETTER);
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

    genPush(program->code, PUSH_REG, 0,                RCX_LETTER); 
    genPush(program->code, PUSH_IMM, 4*curTable->size, 0);
    genOper(program->code, Op_Add);
    genPop (program->code, POP_REG,  0,                RCX_LETTER);

	for (int index = (int) callParam - 1; index >= 0; index--)
	{
        genPop(program->code, POP_IMM_REG_MEM, 4*index, RCX_LETTER);
	}
    
    genJump(program->code, JMP_CALL, program->code->data_size - program->funcArr[func_index].func_start);
    genPush(program->code, PUSH_REG, 0,                RCX_LETTER); 
    genPush(program->code, PUSH_IMM, 4*curTable->size, 0);
    genOper(program->code, Op_Sub);
    genPop (program->code, POP_REG,  0,                RCX_LETTER);

	return OK;
}

size_t CountParams(TreeNode_t* node)
{
	TreeNode_t* curNode = node;
	size_t parCount = 0;

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
		newTable->raxPos = curTable->raxPos + curTable->size; 
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

		Variable_t param = {};
		param.name      = curNode->left->name;
		param.pos       = table->size;
		table->arr[table->size] = param;

		table->size++;
		curNode = curNode->right;
	}

	return parCount;
}

//---------------------------------------------------------------
int ParsePrintf(TreeNode_t* node, Program_t* program) //TODO: lib printf
{
	TreeNode_t* curNode = node;
	while (curNode)
	{
		if (!CheckForVar(node->left, program->tabStk))
		{
			UNDEF_VAR(node->left);
		}
		
		int varPos = GetVarPos(node->left, program->tabStk);
		fprintf(program->asmFile, "PUSH [%d+RAX]\n" 
								  "OUT\n", varPos);

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
		fprintf(program->asmFile, "INP\n" 
								  "POP [%d+RAX]\n", varPos);

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
	
	Variable_t newVar  = {}; 
	VarTable_t* curTable = GetTableFromStk(program->tabStk,  program->tabStk->size - 1);

	newVar.name       = node->left->name;
	newVar.pos        = curTable->size;
		
	curTable->arr[curTable->size] = newVar;
	curTable->size += 1;
		
	CountExpression(node->right, program);

    genPop(program->code, POP_IMM_REG_MEM, newVar.pos * 4, RCX_LETTER);

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
	

    genPop(program->code, POP_IMM_REG_MEM, pos * 4, RCX_LETTER);
	
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
        genPop (program->code, POP_REG,                 0, RCX_LETTER);

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
        genPop (program->code, POP_REG,                 0, RCX_LETTER);
        
        size_t if_addr = program->code->data_size + 7; 
        genJump(program->code, JMP_JMP, 0);

        regenJump(program->code, else_addr);

		ParseStatement(node->right->right, program);
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		VarTableDtor(curTable);
		StackPop(program->tabStk);			
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

        genPush(program->code, PUSH_IMM, curTable->raxPos,          0);
        genPop (program->code, POP_REG,                 0, RCX_LETTER);
        
	    regenJump(program->code, if_addr);	
	}
	
	return OK;
}

int ParseWhile(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}
	
	fprintf(program->asmFile, "\nwhile_%lu:\n", program->whileCounter);
    size_t while_addr = program->code->data_size;
	CountExpression(node->left, program);

    genPush(program->code, PUSH_IMM, 0, 0);
	fprintf(program->asmFile, "PUSH 0\n");

    size_t while_no_addr = program->code->data_size + 7;
    genJump(program->code, JMP_JE, 0);
	fprintf(program->asmFile, "\nJE :while_no_%lu\n", program->whileCounter);

	ParseStatement(node->right, program);
	
	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
	VarTableDtor(curTable);
	StackPop(program->tabStk);			
	
	curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

    genPush(program->code, PUSH_IMM, curTable->raxPos,          0);
    genPop (program->code, POP_REG,                 0, RCX_LETTER);
 
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

	CountExpression(node->left,  program);

	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 2);
	if (isInLocal)
	{
        genPop (program->code, POP_REG,                 0, RAX_LETTER);
        genPush(program->code, PUSH_REG,                0, RCX_LETTER); 
        genPush(program->code, PUSH_IMM, 4*curTable->size,          0);
        genOper(program->code, Op_Sub);
        genPop (program->code, POP_REG,                 0, RCX_LETTER);

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
        genPush(program->code, PUSH_IMM_REG_MEM, varPos, RCX_LETTER);
	}
	else if (node->type == Type_CALL)
	{
		ParseCall(node, program);
        genPush(program->code, PUSH_REG, 0, RAX_LETTER);
	}
    
    #define DEF_OPER(op_type, size, bin_code)               \
            case Op_##op_type:                                \
                binCodeAppend(code, bin_code, size);         \
                fprintf(log_file, "Operation: %s\n", bin_code);\
                break;

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

bin_code_t* binCodeCtor (size_t init_size) {
    if (init_size == 0) return NULL;
	
    bin_code_t* bin_code = (bin_code_t*) calloc(1, sizeof(bin_code_t));
    if (!bin_code) {
        fprintf(stderr, "can't allocate memory for bincode...\n");
        return NULL;
    }

	bin_code->data      = (char*) calloc(init_size, sizeof(char));
    if (!bin_code) {
        fprintf(stderr, "can't allocate memory for bincode...\n");
        return NULL;
    }

	bin_code->size      = init_size;
	bin_code->data_size = 0;

	return bin_code;
}

int binCodeDtor  (bin_code_t* bin_code) {
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

    return 0;
}

static void genRet(bin_code_t* code) {
    binCodeAppend(code, "\xc3", 1);
}


static void genPop(bin_code_t* code,  PopType type, int imm_val,  char reg_val) {
    #define DEF_POP(type, size, bin_code, code)            \
            case POP_##type:                                \
                binCodeAppend(code, bin_code, size);        \
                code                                          \
                fprintf(log_file, "Operation: %s\n", bin_code);\
            break;

    switch(type)
    {
        #include "gencmd.hpp"
		default: break;
    }
    
    #undef DEF_POP
}

static void genPush(bin_code_t* code,  PushType type, int imm_val,  char reg_val) {
    #define DEF_PUSH(type, size, bin_code, code)             \
            case PUSH_##type:                                \
                binCodeAppend(code, bin_code, size);         \
                code                                           \
                fprintf(log_file, "Operation: %s\n", bin_code); \
            break;

    switch(type):
    {
        #include "gencmd.hpp"
		default: break;
    }
    
    #undef DEF_PUSH
}

static void genOper(bin_code_t* code, OperationType type) {
    #define DEF_OPER(op_type, size, bin_code)               \
            case Op_##op_type:                                \
                binCodeAppend(code, bin_code, size);         \
                fprintf(log_file, "Operation: %s\n", bin_code);\
                break;

    switch (type)
	{
        #include "gencmd.hpp"
		default: break;
    }

    #undef DEF_OPER
}

static void genJump(bin_code_t* code, JumpType type, int shift) {
    char jump_arg[4] = {};
    memcpy(jump_arg, &shift, 4);

    #define DEF_JMP(jmp_type, size, bin_code) \
            case JMP_##jmp_type:               \
                binCodeAppend(code, bin_code, size);

    switch(type)
    {
        #include "gencmd.hpp"    
        default:  break;
    }

    #undef DEF_JMP

    binCodeAppend(code, jump_arg, 4);
}

static void regenJump(bin_code_t* code, int arg) {
    int shift = code->data_size - arg; 
    memcpy(code->data + arg, &shift, sizeof(int));
}












