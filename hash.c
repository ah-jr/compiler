// Implementação baseada na versão vista em aula

#include "hash.h"

HASH_NODE *Table[HASH_SIZE];

void hashInit()
{
    int i;
    for(i=0; i<HASH_SIZE; i++)
        Table[i] = 0;
}

int hashAddress(char *text)
{
    int address = 1;
    int i;
    for(i=0; i<strlen(text); i++)
        address = (address * text[i]) % 
            HASH_SIZE + 1;
    return address - 1;
}

HASH_NODE *hashFind(char *text)
{
    HASH_NODE *node;
    int address = hashAddress(text);
    for(node=Table[address]; node; node = node->next)
        if(strcmp(node->text,text)==0)
            return node;
    return 0;
}

HASH_NODE *hashInsert(char *text, int type)
{
    HASH_NODE *newNode;
    int address = hashAddress(text);

    if((newNode = hashFind(text)) != 0)
        return newNode;
    
    newNode  = (HASH_NODE*) calloc(1, sizeof(HASH_NODE));
    newNode->type = type;
    newNode->auxnum = 0;
    newNode->text = (char*)calloc(strlen(text)+1,sizeof(char));
    strcpy(newNode->text, text);
    newNode->next = Table[address];
    newNode->aux  = 0;
    Table[address] = newNode;

    return newNode;
}

void hashPrint()
{
    int i;
    HASH_NODE *node;
    for(i=0; i<HASH_SIZE; ++i)
        for(node=Table[i]; node; node = node->next)
            printf("Table[%d] has %s, type %d, datatype %d\n", i, node->text, node->type, node->datatype);
}

int hash_check_undeclared()
{
    int undeclared = 0;

    int i;
    HASH_NODE *node;
    for(i=0; i<HASH_SIZE; ++i)
        for(node=Table[i]; node; node = node->next)
            if (node->type == SYMBOL_IDENTIFIER)
            {
                ++undeclared;
                fprintf(stderr, "Semantic Error: Identifier %s undeclared\n", node->text);
            }

    return undeclared;            
}

HASH_NODE *makeTemp(void){
    static int serial = 0;
    char buffer[256] = "";

    sprintf(buffer, "_reserved_name_reg_%d", serial++);
    return hashInsert(buffer, SYMBOL_VARIABLE);
}

HASH_NODE* makeLabel(){
    static int serial = 0;
    char buffer[256] = "";

    sprintf(buffer, "_reserved_name_lbl_%d", serial++);
    return hashInsert(buffer, SYMBOL_LABEL);    
}

HASH_NODE* makeBeginFunctionLabel(const char *name){
    char buffer[256] = "";

    sprintf(buffer, "_reserved_name_function_%s", name);
    return hashInsert(buffer, SYMBOL_FUNCTION_CALL_LABEL);    
}

void printASM(FILE *fout){
    int i;
    HASH_NODE *node;

    for (i=0; i<HASH_SIZE; ++i)
        for (node=Table[i]; node; node = node->next)
            if(node->type == SYMBOL_VARIABLE)
                if (node->auxnum > 0){
                    for (int index = 0; index < node->auxnum ; index++){
                        fprintf(fout, "_%s_%d: \t.long \t%d\n", node->text, index, node->inits[index]);
                    }
                }
                else fprintf(fout, "_%s: \t.long \t%d\n", node->text, node->init);
}