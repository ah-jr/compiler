// Implementação baseada na versão vista em aula

#ifndef HASH_HEADER
#define HASH_HEADER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SYMBOL_IDENTIFIER 1
#define SYMBOL_INTEGER    2   
#define SYMBOL_CHAR       3   
#define SYMBOL_STRING     4
#define SYMBOL_VARIABLE   5
#define SYMBOL_FUNCTION   6
#define SYMBOL_FLOAT      7
#define SYMBOL_LABEL      8
#define SYMBOL_FUNCTION_CALL_LABEL 9
#define SYMBOL_SPECIAL_WHILE 10


#define DATATYPE_INT   1
#define DATATYPE_FLOAT 2
#define DATATYPE_CHAR  3

#define HASH_SIZE 997

typedef struct hash_node
{
    int type;
    int datatype;
    int auxnum;
    int init;
    int inits[50];
    char *text;
    struct hash_node* next;
    struct hash_node* aux;
} HASH_NODE;

void hashInit(void);
int hashAddress(char *text);
HASH_NODE *hashFind(char *text);
HASH_NODE *hashInsert(char *text,int type);
void hashPrint(void);
int hash_check_undeclared(void);
HASH_NODE *makeTemp(void);
HASH_NODE* makeLabel(void);
HASH_NODE* makeBeginFunctionLabel(const char *name);

void printASM(FILE *fout);

#endif