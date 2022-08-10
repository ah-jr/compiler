#include "tacs.h"

// Funções usadas localmente
TAC* makeIf(TAC* code0, TAC* code1);
TAC* makeIfElse(TAC* code0, TAC* code1, TAC* code2);
TAC* makeBasicOp(TAC* code0, TAC* code1, int type);
TAC* makeFunctionDec(TAC* name, TAC* params, TAC* content);
TAC* makeFunctionCall(TAC* name, TAC* params);
TAC* makeTacPrint(TAC* params);
TAC* makeTacWhile(TAC* code0, TAC* code1);
TAC* makeTacWhileOpt(AST *node);

HASH_NODE* getRes(TAC* code);

// Criação da TAC
TAC* tacCreate(int type, HASH_NODE* res, HASH_NODE* op1, HASH_NODE* op2){
    TAC* newTac = 0;
    newTac = (TAC*) calloc(1, sizeof(TAC));
    newTac->type = type;
    newTac->res = res;
    newTac->op1 = op1;
    newTac->op2 = op2;

    return newTac;
}

// Print dos nodos TAC
void tacPrint(TAC* tac)
{
    if (!tac) return;
    
    switch (tac->type )
    {
        // Não precisamos printar os símbolos!
        case TAC_SYMBOL: /*tacPrintNode(tac, "TAC_SYMBOL"); */break;
        case TAC_COPY: tacPrintNode(tac, "TAC_COPY"); break;

        case TAC_ADD: tacPrintNode(tac, "TAC_ADD"); break;
        case TAC_SUB: tacPrintNode(tac, "TAC_SUB"); break;
        case TAC_MUL: tacPrintNode(tac, "TAC_MUL"); break;
        case TAC_DIV: tacPrintNode(tac, "TAC_DIV"); break;
        case TAC_EQ: tacPrintNode(tac, "TAC_EQ"); break;
        case TAC_DIF: tacPrintNode(tac, "TAC_DIF"); break;
        case TAC_G: tacPrintNode(tac, "TAC_G"); break;
        case TAC_L: tacPrintNode(tac, "TAC_L"); break;
        case TAC_GE: tacPrintNode(tac, "TAC_GE"); break;
        case TAC_LE: tacPrintNode(tac, "TAC_LE"); break;

        case TAC_JF: tacPrintNode(tac, "TAC_JZ"); break;
        case TAC_JUMP: tacPrintNode(tac, "TAC_JUMP"); break;

        case TAC_BEGINFUN: tacPrintNode(tac, "TAC_BEGINFUN"); break;
        case TAC_ENDFUN: tacPrintNode(tac, "TAC_ENDFUN"); break;

        case TAC_LABEL: tacPrintNode(tac, "TAC_LABEL"); break;
        case TAC_ARRAY: tacPrintNode(tac, "TAC_ARRAY"); break;
        case TAC_ARRAY_ASSIGN: tacPrintNode(tac, "TAC_ARRAY_ASSIGN"); break;
        case TAC_CALL: tacPrintNode(tac, "TAC_CALL"); break;
        case TAC_ARG: tacPrintNode(tac, "TAC_ARG"); break;
        case TAC_PARAM: tacPrintNode(tac, "TAC_PARAM"); break;
        case TAC_AFTER_PARAM: tacPrintNode(tac, "TAC_AFTER_PARAM"); break;
        case TAC_PRINTARG: tacPrintNode(tac, "TAC_PRINTARG"); break;
        case TAC_READ: tacPrintNode(tac, "TAC_READ"); break;
        case TAC_PRINT: tacPrintNode(tac, "TAC_PRINT"); break;
        case TAC_RET: tacPrintNode(tac, "TAC_RET"); break;

        default: tacPrintNode(tac, "TAC_DEFAULT"); break;
    }
}

void tacPrintNode(TAC* tac, const char *tacName)
{
    fprintf(stderr, "TAC(");
    fputs(tacName, stderr);
    fprintf(stderr, ", %s", (tac->res) ? tac->res->text : "0");
    fprintf(stderr, ", %s", (tac->op1) ? tac->op1->text : "0");
    fprintf(stderr, ", %s", (tac->op2) ? tac->op2->text : "0");
    fprintf(stderr, ");\n");
}

void tacPrintBackwards(TAC* tac){
    if (!tac) return;
    else{
        tacPrintBackwards(tac->prev);
        tacPrint(tac);
    }
}

// Garantir que retorno do Res está correto
HASH_NODE* getRes(TAC* code){
    return code ? code->res : 0;
}

// Geração de código 
TAC* generateCode(AST* node){
    int i;
    TAC *result = 0;
    TAC *code[MAX_SONS];

    if (!node) return 0;

    for (i = 0; i< MAX_SONS; i++)   
        code[i] = generateCode(node->son[i]);

    switch(node->type)
    {
        // Acessos e declarações
        case AST_SYMBOL: result = tacCreate(TAC_SYMBOL, node->symbol, 0, 0); break;
        case AST_STRING: result = tacCreate(TAC_SYMBOL, node->symbol, 0, 0); break;

        case AST_ARRAY_ACCESS:
            result = tacJoin(code[1], tacCreate(TAC_ARRAY, makeTemp(), getRes(code[0]), getRes(code[1])));
            break;

        case AST_ARRAY_ASSIGN: 
            result = tacJoin(tacJoin(tacJoin(code[0], code[1]), code[2]), tacCreate(TAC_ARRAY_ASSIGN, getRes(code[0]), getRes(code[1]), getRes(code[2])));
            break;

        case AST_ASSIGN: 
            result = tacJoin(tacJoin(code[0], code[1]), tacCreate(TAC_COPY, getRes(code[0]), getRes(code[1]), 0));
            break;

        // Operações básicas
        case AST_ADD: result = makeBasicOp(code[0], code[1], TAC_ADD); break;         
        case AST_SUB: result = makeBasicOp(code[0], code[1], TAC_SUB); break;   
        case AST_MUL: result = makeBasicOp(code[0], code[1], TAC_MUL); break;   
        case AST_DIV: result = makeBasicOp(code[0], code[1], TAC_DIV); break;   

        case AST_EQ: result = makeBasicOp(code[0], code[1], TAC_EQ); break;   
        case AST_DIF: result = makeBasicOp(code[0], code[1], TAC_DIF); break;   
        case AST_G: result = makeBasicOp(code[0], code[1], TAC_G); break;   
        case AST_L: result = makeBasicOp(code[0], code[1], TAC_L); break;   
        case AST_GE: result = makeBasicOp(code[0], code[1], TAC_GE); break;   
        case AST_LE: result = makeBasicOp(code[0], code[1], TAC_LE); break;  
         
        // Tratar condicionais de acordo com o tipo
        case AST_IF: 
            if (node->son[2]) result = makeIfElse(code[0], code[1], code[2]);
            else              result = makeIf(code[0], code[1]); 
            break;

        case AST_FUNCTION_CALL: result = makeFunctionCall(code[0], code[1]); break; 
        case AST_FUNCTION_DEC : result = makeFunctionDec(code[1], code[2], code[3]); break; 
        case AST_FUNCTION_DEC_PARAM : 
            result = tacJoin(tacCreate(TAC_PARAM, getRes(code[1]), 0, 0), tacJoin(code[0], tacJoin(code[1], code[2])));
            break;

       // TAC para argumentos é a mesma para funções e print
        case AST_FUNCTION_CALL_PARAM : 
            result = tacJoin(tacCreate(TAC_ARG, getRes(code[0]), 0, 0), tacJoin(code[0], code[1])); 
            break;
        case AST_PRINT_PARAM_STRING : 
        case AST_PRINT_PARAM_EXP : result = tacJoin(tacJoin(code[0], code[1]), tacCreate(TAC_PRINTARG, getRes(code[0]), 0, 0)); break;
        
        case AST_BODY : result = code[0] ? code[0] : 0; break;  

        case AST_READ : result = tacCreate(TAC_READ, makeTemp(), 0, 0); break;
        case AST_PRINT : result = makeTacPrint(code[0]); break;
        case AST_RETURN : result = tacJoin(code[0], tacCreate(TAC_RET, getRes(code[0]), 0, 0)); break;
        case AST_WHILE : 
            if (node->symbol->auxnum == 0) result = makeTacWhile   (code[0], code[1]);
            else                           result = makeTacWhileOpt(node);
            break;

        default:
            result = tacJoin(code[0], tacJoin(code[1], tacJoin(code[2], code[3])));
            break;
    }
    return result;
}

// Juntar as TACS
TAC* tacJoin(TAC* l1, TAC* l2){
    if(!l1) return l2;
    if(!l2) return l1;

    TAC* aux;  

    for(aux = l2; aux->prev != 0; aux = aux->prev);

    aux->prev = l1;
    return l2;
}

// Tratar operações aritméticas e lógicas simples
TAC* makeBasicOp(TAC* code0, TAC* code1, int type){
    return tacJoin(tacJoin(code0, code1), tacCreate(type, makeTemp(), getRes(code0), getRes(code1)));
}

TAC* makeIf(TAC* code0, TAC* code1)
{
    TAC *jumptac = 0;
    TAC *labeltac = 0;
    HASH_NODE *newlabel = 0;

    newlabel = makeLabel();

    jumptac = tacCreate(TAC_JF, newlabel, getRes(code0), 0);
    jumptac->prev = code0;

    labeltac = tacCreate(TAC_LABEL, newlabel, 0,0);
    labeltac->prev = code1;

    return tacJoin(jumptac, labeltac);
}

// Coloca um JZ para uma Label no else, e um jump normal no final do
// then para um Label depois do else

/* Exemplo:

TAC(TAC_JZ, _reserved_name_lbl_0, X, 0);

**CORPO DO THEN**

TAC(TAC_JUMP, _reserved_name_lbl_1, 0, 0);
TAC(TAC_LABEL, _reserved_name_lbl_0, 0, 0);

**CORPO DO ELSE**

TAC(TAC_LABEL, _reserved_name_lbl_1, 0, 0);
*/

TAC* makeIfElse(TAC* code0, TAC* code1, TAC* code2)
{
    TAC *jztac = 0;
    TAC *jumptac = 0;
    TAC *labeltacjz = 0;
    TAC *labeltacjump = 0;

    TAC *aux1 = 0;
    
    HASH_NODE *newlabeljz = 0;
    HASH_NODE *newlabeljump = 0;

    newlabeljz = makeLabel();
    newlabeljump = makeLabel();

    jztac = tacCreate(TAC_JF, newlabeljz, getRes(code0), 0);
    jztac->prev = code0;

    jumptac = tacCreate(TAC_JUMP, newlabeljump, 0, 0);
    jumptac->prev = code1;

    aux1 = tacJoin(jztac, jumptac);

    labeltacjz = tacCreate(TAC_LABEL, newlabeljz, 0,0);
    labeltacjz->prev = aux1;

    labeltacjump = tacCreate(TAC_LABEL, newlabeljump, 0,0);
    labeltacjump->prev = code2;

    return tacJoin(labeltacjz, labeltacjump);
}

TAC* makeFunctionDec(TAC* name, TAC* params, TAC* content){
    TAC *beginfun = 0;
    TAC *endfun = 0;
    TAC *aux = 0;

    HASH_NODE *functionLabel = 0;

    // Cria uma "label" para a chamada da função, que também é colocada na hash 
    // e pode ser utilizada depois na chamada
    functionLabel = makeBeginFunctionLabel(name->res->text);

    beginfun = tacCreate(TAC_BEGINFUN, functionLabel, 0, 0);
    beginfun->prev = name;
    aux = tacCreate(TAC_AFTER_PARAM, 0, 0, 0);

    if (params) {
        aux->prev = params;
        aux = tacJoin(aux, content);
       
        while (params->prev) params = params->prev;
        params->prev = beginfun;
    }
    else{
        aux->prev = beginfun;
        aux = tacJoin(aux, content);
    }

    endfun = tacCreate(TAC_ENDFUN, functionLabel, 0, 0);
    endfun->prev = aux;

    return (endfun);
}

TAC* makeFunctionCall(TAC* name, TAC* params){
    TAC *result = 0;
    result = tacCreate(TAC_CALL, makeTemp(), getRes(name), 0);

    if (params){
        result->prev = params;
    }

    return result; 
}

TAC* makeTacPrint(TAC* params){
    TAC *result = 0;
    result = tacCreate(TAC_PRINT, 0, 0, 0);

    result->prev = params;
    return result;
}

// Coloca um JZ que sai do loop, e um jump normal no final do
// corpo do laço que sempre volta para o JZ; 

/* Exemplo:

TAC(TAC_LABEL, _reserved_name_lbl_1, 0, 0);
TAC(TAC_JZ, _reserved_name_lbl_0, X, 0);

**CORPO DO LAÇO**

TAC(TAC_JUMP, _reserved_name_lbl_1, 0, 0);
TAC(TAC_LABEL, _reserved_name_lbl_0, 0, 0);
*/

TAC* makeTacWhile(TAC* code0, TAC* code1){
    TAC *jztac = 0;
    TAC *jumptac = 0;
    TAC *labeltacjz = 0;
    TAC *labeltacjump = 0;
    TAC *aux1 = 0;
    
    HASH_NODE *newlabeljz = 0;
    HASH_NODE *newlabeljump = 0;

    newlabeljz = makeLabel();
    newlabeljump = makeLabel();

    labeltacjump = tacCreate(TAC_LABEL, newlabeljump, 0,0);
    tacJoin(labeltacjump, code0);

    jztac = tacCreate(TAC_JF, newlabeljz, getRes(code0), 0);
    jztac->prev = code0;

    jumptac = tacCreate(TAC_JUMP, newlabeljump, 0, 0);
    jumptac->prev = code1;

    aux1 = tacJoin(jztac, jumptac);

    labeltacjz = tacCreate(TAC_LABEL, newlabeljz, 0,0);
    labeltacjz->prev = aux1;

    return labeltacjz;
}

// TAC While otimizado:
// Quando um while contiver um número constante de repetições, 
// vamos fazer o unrolling, ou seja, escrever o código repeti-
// damente na saída em Assembly ao invés de utilizar funções
// de JUMP para navegar no laço.

TAC* makeTacWhileOpt(AST *node){
    int rep = node->symbol->auxnum;
    TAC* body = 0;
    TAC* last = 0;

    for (int i = 0; i < rep; i++)
    {
        body = generateCode(node->son[1]);
        last = tacJoin(body, last);          
    }

    return last;
}

