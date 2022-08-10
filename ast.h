// AST - Abstract Syntax Tree

#include "hash.h"

#ifndef AST_HEADER
#define AST_HEADER

#define MAX_SONS 4


#define AST_SYMBOL              1
#define AST_INT                 2
#define AST_CHAR                3
#define AST_FLOAT               4
#define AST_STRING              5
#define AST_LIST                6
#define AST_ARRAY_ACCESS        7

#define AST_ASSIGN              8

#define AST_ADD                 9
#define AST_SUB                 10
#define AST_MUL                 11
#define AST_DIV                 12
#define AST_L                   13 
#define AST_G                   14
#define AST_LE                  15
#define AST_GE                  16
#define AST_EQ                  17
#define AST_DIF                 18

#define AST_IF                  19                
#define AST_WHILE               20       
#define AST_GOTO                21       
#define AST_READ                22       
#define AST_PRINT               23       
#define AST_RETURN              24 
#define AST_LABEL               25 
#define AST_BODY                26
#define AST_CAPSULE             27

#define AST_DECLARATION         28
#define AST_COMMAND_LIST        29
#define AST_COMMAND_LIST_LABEL  30
#define AST_VAR                 31
#define AST_VAR_FLOAT           32
#define AST_VAR_ARRAY           33
#define AST_VAR_ARRAY_VALUE     34

#define AST_FUNCTION_DEC        35
#define AST_FUNCTION_CALL       36
#define AST_PRINT_PARAM_STRING  37
#define AST_PRINT_PARAM_EXP     38

#define AST_FUNCTION_CALL_PARAM 39
#define AST_FUNCTION_DEC_PARAM  40

#define AST_SIZE                41
#define AST_ARRAY_ASSIGN        42

typedef struct astnode
{
    int type;
    HASH_NODE *symbol;
    struct astnode *son[MAX_SONS];
} AST;

AST *astCreate(int type, HASH_NODE *symbol, AST* s0, AST* s1, AST* s2, AST* s3);
void astPrint(AST *node, int level);
void astDecompile(AST* node, FILE* out);


#endif
 
