#include "tacs.h"
#include <string.h>

#define PT_STRING 0
#define PT_INT    1
#define PT_VAR    2

typedef struct printArgList {
    int type;
    int value;
    int index;
    char *text;
    struct printArgList *next;
} PRINT_ARG_LIST;

TAC* tacReverse(TAC *tac);
void generateASM(TAC* first);
