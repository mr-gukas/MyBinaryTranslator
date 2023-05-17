#pragma once
#include "stack/stack.h"
#include "../frontend/frontend.h"
#include "../tree/tree.h"
#include "opcodes.h"
#include <stdint.h>
#include <elf.h>
#include <sys/mman.h>
#include <limits.h>    
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define AST_FILE_NAME "obj/standart.txt"

#define SAVEREGS        asm("push %rax\n"    \
                            "push %rsi\n"    \
                            "push %rdi\n"    \
                            "push %rbp\n");


#define RESTOREREGS         asm("pop %rbp\n\t"   \
                                "pop %rdi\n\t"   \
                                "pop %rsi\n\t"   \
                                "pop %rax\n\t");

#ifdef DEBUG_MODE
    #define LOG_PRINT(...) fprintf(LOG_FILE, __VA_ARGS__);
#else
    #define LOG_PRINT(...) ;
#endif

#define UNDEF_VAR(node)                                                        \
  fprintf(stderr, "An uninitialized variable was encountered: <%s>\n",         \
          node->name);                                                         \
  abort();

#define UNDEF_FUNC(node)                                                       \
  fprintf(stderr, "An unknown function encountered: <%s>\n", node->name);      \
  abort();

#define ALREADY_DEF_VAR(node)                                                  \
  fprintf(stderr, "The variable has already been declared: <%s>\n",            \
          node->name);                                                         \
  abort();


const size_t QWORD      = 8;
const size_t PAGE_SIZE  = 4096;
const size_t RAM_SIZE   = 256;
const size_t VAR_COUNT  = 10000;
const size_t FUNC_COUNT = 10000;
const size_t POISON_PTR = 31415;
const size_t PERLINE    = 16;

enum RetValue { VOID, INT };

struct Variable_t {
  char *name;
  size_t pos;
};

struct VarTable_t {
  Variable_t *arr;
  size_t size;

  size_t raxPos;
};

struct bin_code_t {
    size_t size;       // размер выделенной памяти
    size_t data_size;  // фактический размер хранящихся данных
    char*  data;       // указатель на данные
};

struct Func_t {
    char     *name;
    size_t   parCount;
    RetValue retVal;
    size_t   func_start;
};

struct Program_t {
    TreeNode_t *node;
    Stack_t    *tabStk;

    Func_t *funcArr;
    size_t funcCount;
    
    bin_code_t* code;
    char*       ram_arr;

    int inMain;
};


const char reg_name[4][4] = {"RAX", "RCX", "RDX", "RDX"};

 const char RAX_MASK = 0x0;
 const char RBX_MASK = 0x3; 
 const char RCX_MASK = 0x1; 
 const char RDX_MASK = 0x2; 

enum PopType {
    POP_REG = 0,
    POP_REG_MEM,
    POP_IMM_MEM,
    POP_IMM_REG_MEM,
};

enum PushType {
    PUSH_IMM = 0,
    PUSH_REG,
    PUSH_IMM_MEM,
    PUSH_REG_MEM,
    PUSH_IMM_REG_MEM,
};

enum JumpType {
    JMP_JMP = 0, 
    JMP_JE,
    JMP_JNE,
    JMP_JG,
    JMP_JGE,
    JMP_JL, 
    JMP_JLE,
    JMP_CALL,
};

enum ProgramStatus {
  OK = 0,
  WRONG_DATA,
  NULL_TREE,
  UNDEF_VAR,
  ALREADY_DEF_VAR,
  ALREADY_DEF_FUNC,
  UNDEF_FUNC,
  NO_MEM,
  BAD_CALL,
};

int readAST(Tree_t* tree, const char* ast_file);

bin_code_t* binCodeCtor();

int binCodeDtor(bin_code_t* bin_code);
int binCodeAppend(bin_code_t* bin_code, const char* data, size_t data_size);

static void genRet (bin_code_t* code);
static void genPop (bin_code_t* code,  PopType type, int imm_val, char reg_val);
static void genPush(bin_code_t* code, PushType type, int imm_val, char reg_val);
static void genOper(bin_code_t* code, OperationType type);
static void genJump(bin_code_t* code, JumpType type, int shift);
static void regenJump(bin_code_t* code, int arg);

int GetVarPos(TreeNode_t *node, Stack_t *stk);
VarTable_t *GetTableFromStk(Stack_t *stk, size_t index);
int CheckForVar(TreeNode_t *node, Stack_t *stk);
void CountExpression(TreeNode_t *node, Program_t *program);
int ParseRet(TreeNode_t *node, Program_t *program);
int ParseWhile(TreeNode_t *node, Program_t *program);
int ParseIf(TreeNode_t *node, Program_t *program);
int ParseAssign(TreeNode_t *node, Program_t *program);
int ParseDef(TreeNode_t *node, Program_t *program);
size_t ParseParams(TreeNode_t *node, VarTable_t *table);
int ParseCall(TreeNode_t *node, Program_t *program);
int ParseFunc(TreeNode_t *node, Program_t *program);
void ParseStatement(TreeNode_t *node, Program_t *program);
int ParseScanf(TreeNode_t *node, Program_t *program);
int ParsePrintf(TreeNode_t *node, Program_t *program);
int ProgramCtor(Program_t *program, TreeNode_t *node);
int ProgramDtor(Program_t* program);

Program_t* WriteBinCode(TreeNode_t *root);
VarTable_t *VarTableCtor(Stack_t *stk);
int         VarTableDtor(VarTable_t *table);
int         FuncArrDtor(Func_t *funcArr);
size_t      CountParams(TreeNode_t *node);
Func_t *FuncArrCtor();
static void binCodeRun(bin_code_t* code);


void Printf4Translator(int num);
void Scanf4Translator(int* num);
static void binCodeMemcpy(bin_code_t* code, size_t dest, void* arg, size_t size);
void hexDump (const void * addr, const int len, FILE* file);
















