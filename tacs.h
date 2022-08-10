#ifndef TACS_HEADER
#define TACS_HEADER

#include "ast.h"

#define TAC_SYMBOL 1

#define TAC_ADD 2
#define TAC_SUB 3
#define TAC_MUL 4
#define TAC_DIV 5

#define TAC_EQ 6
#define TAC_DIF 7
#define TAC_G 8
#define TAC_L 9
#define TAC_GE 10
#define TAC_LE 11

#define TAC_COPY 12
#define TAC_JF 13
#define TAC_JUMP 14
#define TAC_LABEL 15

#define TAC_BEGINFUN 16
#define TAC_ENDFUN 17

#define TAC_ARRAY 18

#define TAC_CALL 19
#define TAC_ARG 20

#define TAC_READ 21
#define TAC_PRINT 22
#define TAC_RET 23

#define TAC_PRINTARG 24
#define TAC_PARAM 25
#define TAC_AFTER_PARAM 26

#define TAC_ARRAY_ASSIGN 27

typedef struct tac_node{
    int type;
    HASH_NODE * res;
    HASH_NODE * op1;
    HASH_NODE * op2;
    struct tac_node * prev;
    struct tac_node * next;
} TAC;

// TACs

TAC* tacCreate(int type, HASH_NODE* res, HASH_NODE* op1, HASH_NODE* op2);
void tacPrint(TAC* tac);
void tacPrintNode(TAC* tac, const char* tacName);
void tacPrintBackwards(TAC* tac);
TAC* tacJoin(TAC* l1, TAC* l2);

// Geração de código
 
TAC* generateCode(AST* node);

#endif