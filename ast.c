#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

AST *astCreate(int type, HASH_NODE *symbol, AST* s0, AST* s1, AST* s2, AST* s3)
{
    AST *newnode;

    newnode = (AST *) calloc(1,sizeof(AST));

    newnode->type = type;
    newnode->symbol = symbol;
    newnode->son[0] = s0;
    newnode->son[1] = s1;
    newnode->son[2] = s2;
    newnode->son[3] = s3;

    return newnode;
}

void astPrint(AST *node, int level)
{
    if (node == 0){
        return;
    }

    for (int i =0; i < level; ++i){
        fprintf(stderr, "  ");
    }

    fprintf(stderr,"ast: ");
    switch(node->type){
        case AST_SYMBOL              : fprintf(stderr, "AST_SYMBOL"); break;
        case AST_INT                 : fprintf(stderr, "AST_INT"); break;
        case AST_CHAR                : fprintf(stderr, "AST_CHAR"); break;
        case AST_FLOAT               : fprintf(stderr, "AST_FLOAT"); break;
        case AST_STRING              : fprintf(stderr, "AST_STRING"); break; 
        case AST_LIST                : fprintf(stderr, "AST_LIST"); break;
        case AST_ARRAY_ACCESS        : fprintf(stderr, "AST_ARRAY_ACCESS"); break;
        case AST_ASSIGN              : fprintf(stderr, "AST_ASSIGN"); break;
        case AST_ARRAY_ASSIGN        : fprintf(stderr, "AST_ARRAY_ASSIGN"); break;
        case AST_ADD                 : fprintf(stderr, "AST_ADD"); break;
        case AST_SUB                 : fprintf(stderr, "AST_SUB"); break;
        case AST_MUL                 : fprintf(stderr, "AST_MUL"); break;
        case AST_DIV                 : fprintf(stderr, "AST_DIV"); break;
        case AST_L                   : fprintf(stderr, "AST_L"); break; 
        case AST_G                   : fprintf(stderr, "AST_G"); break;
        case AST_LE                  : fprintf(stderr, "AST_LE"); break;
        case AST_GE                  : fprintf(stderr, "AST_GE"); break;
        case AST_EQ                  : fprintf(stderr, "AST_EQ"); break;
        case AST_DIF                 : fprintf(stderr, "AST_DIF"); break;
        case AST_IF                  : fprintf(stderr, "AST_IF"); break;         
        case AST_WHILE               : fprintf(stderr, "AST_WHILE"); break;       
        case AST_GOTO                : fprintf(stderr, "AST_GOTO"); break;       
        case AST_READ                : fprintf(stderr, "AST_READ"); break;       
        case AST_PRINT               : fprintf(stderr, "AST_PRINT"); break;       
        case AST_RETURN              : fprintf(stderr, "AST_RETURN"); break; 
        case AST_LABEL               : fprintf(stderr, "AST_LABEL"); break; 
        case AST_BODY                : fprintf(stderr, "AST_BODY"); break;
        case AST_CAPSULE             : fprintf(stderr, "AST_CAPSULE"); break;
        case AST_DECLARATION         : fprintf(stderr, "AST_DECLARATION"); break;
        case AST_COMMAND_LIST        : fprintf(stderr, "AST_COMMAND_LIST"); break;
        case AST_VAR                 : fprintf(stderr, "AST_VAR"); break;
        case AST_VAR_FLOAT           : fprintf(stderr, "AST_VAR_FLOAT"); break;
        case AST_VAR_ARRAY           : fprintf(stderr, "AST_VAR_ARRAY"); break;
        case AST_VAR_ARRAY_VALUE     : fprintf(stderr, "AST_VAR_ARRAY_VALUE"); break;
        case AST_FUNCTION_DEC        : fprintf(stderr, "AST_FUNCTION_DEC"); break;
        case AST_FUNCTION_CALL       : fprintf(stderr, "AST_FUNCTION_CALL"); break;
        case AST_PRINT_PARAM_STRING  : fprintf(stderr, "AST_PRINT_PARAM_STRING"); break;
        case AST_PRINT_PARAM_EXP     : fprintf(stderr, "AST_PRINT_PARAM_EXP"); break;
        case AST_COMMAND_LIST_LABEL  : fprintf(stderr, "AST_COMMAND_LIST_LABEL"); break;
        case AST_FUNCTION_CALL_PARAM : fprintf(stderr, "AST_FUNCTION_CALL_PARAM"); break;
        case AST_FUNCTION_DEC_PARAM  : fprintf(stderr, "AST_FUNCTION_DEC_PARAM"); break;
        default: fprintf(stderr, "AST_UNKNOWN"); break;
    }
    if(node->symbol != 0){
        fprintf(stderr, ",%s\n", node->symbol->text);
    }
    else{
        fprintf(stderr, ",0\n");
    }

    for (int i = 0; i< MAX_SONS; i++){
        astPrint(node->son[i], level + 1);
    }
}

void astDecompile(AST* node, FILE* out)
{
    if(node == NULL){
        return;
    }

    switch(node->type){
        case AST_SYMBOL       : fprintf(out, "%s", node->symbol->text); break;
        case AST_INT          : fprintf(out, "int"); break;
        case AST_CHAR         : fprintf(out, "char"); break;
        case AST_FLOAT        : fprintf(out, "float"); break;
        case AST_STRING       : break;
        case AST_LIST:          fprintf(out, "%s", node->son[0]->symbol->text); 
                                if(node->son[1] == NULL) fprintf(out,";\n"); 
                                else
                                {
                                    fprintf(out, " ");
                                    astDecompile(node->son[1], out);
                                }
                                break;                           
        case AST_ARRAY_ACCESS:  fprintf(out, "%s[", node->son[0]->symbol->text); 
                                astDecompile(node->son[1], out); fprintf(out, "]");
                                break;                 
        case AST_ASSIGN:        fprintf(out, "%s", node->symbol->text); 
                                fprintf(out, " = "); 
                                astDecompile(node->son[0], out);
                                break;
        case AST_ARRAY_ASSIGN:  fprintf(out, "%s[", node->symbol->text); 
                                astDecompile(node->son[0], out); fprintf(out, "]");
                                fprintf(out, " = "); 
                                astDecompile(node->son[1], out);
                                break;
	    case AST_ADD          : astDecompile(node->son[0], out); 
                                fprintf(out, " + ");  
                                astDecompile(node->son[1], out);
                                break;
        case AST_SUB          : astDecompile(node->son[0], out); 
                                fprintf(out, " - ");  
                                astDecompile(node->son[1], out); 
                                break;
        case AST_MUL          : astDecompile(node->son[0], out); 
                                fprintf(out, " * "); 
                                astDecompile(node->son[1], out); 
                                break;
        case AST_DIV          : astDecompile(node->son[0], out); 
                                fprintf(out, " / ");  
                                astDecompile(node->son[1], out); 
                                break;
        case AST_L            : astDecompile(node->son[0], out); 
                                fprintf(out, " < ");  
                                astDecompile(node->son[1], out); 
                                break;
        case AST_G            : astDecompile(node->son[0], out); 
                                fprintf(out, " > ");  
                                astDecompile(node->son[1], out); 
                                break;
        case AST_LE           : astDecompile(node->son[0], out); 
                                fprintf(out, " <= "); 
                                astDecompile(node->son[1], out); 
                                break;
        case AST_GE           : astDecompile(node->son[0], out);
                                fprintf(out, " >= "); 
                                astDecompile(node->son[1], out); 
                                break;
        case AST_EQ           : astDecompile(node->son[0], out); 
                                fprintf(out, " == "); 
                                astDecompile(node->son[1], out); 
                                break;
        case AST_DIF          : astDecompile(node->son[0], out); 
                                fprintf(out, " != "); 
                                astDecompile(node->son[1], out); 
                                break;
        case AST_IF:            fprintf(out, "if "); 
                                astDecompile(node->son[0], out); 
                                fprintf(out, " then\n"); 
                                astDecompile(node->son[1], out);
                                if(node->son[2] != NULL)
                                {
                                    fprintf(out, "\nelse\n");
                                    astDecompile(node->son[2], out);
                                }
                                break;
        case AST_WHILE:         fprintf(out, "while "); 
                                astDecompile(node->son[0], out); 
                                fprintf(out, "\n"); 
                                astDecompile(node->son[1], out); 
                                break;
        case AST_GOTO:          fprintf(out, "goto %s", node->son[0]->symbol->text); 
                                break;
        case AST_READ:          fprintf(out, "read");
                                break;
        case AST_PRINT:         fprintf(out, "print "); 
                                astDecompile(node->son[0], out); 
                                break;
        case AST_RETURN:        fprintf(out, "return ");
                                astDecompile(node->son[0], out);
                                break;
        case AST_LABEL:         fprintf(out, "%s:\n", node->son[0]->symbol->text); 
                                break;
        case AST_BODY:          fprintf(out, "{\n");
                                astDecompile(node->son[0], out);
                                fprintf(out, "}");
                                break;                                
        case AST_CAPSULE:       fprintf(out, "("); 
                                astDecompile(node->son[0], out); 
                                fprintf(out, ")"); 
                                break;
        case AST_DECLARATION  : astDecompile(node->son[0], out); 
                                astDecompile(node->son[1], out); 
                                break;
        case AST_COMMAND_LIST:  astDecompile(node->son[0], out);
                                fprintf(out, ";\n");
                                astDecompile(node->son[1], out);
                                break;
        case AST_COMMAND_LIST_LABEL: 
                                astDecompile(node->son[0], out); 
                                astDecompile(node->son[1], out);
                                break;
        case AST_VAR:           astDecompile(node->son[0], out);
                                fprintf(out, " %s: %s;\n", node->son[1]->symbol->text, node->son[2]->symbol->text); 
                                break;
        case AST_VAR_FLOAT:     astDecompile(node->son[0], out);
                                fprintf(out, " %s: %s / %s;\n", node->son[1]->symbol->text, node->son[2]->symbol->text, node->son[3]->symbol->text); 
                                break;                                
        case AST_VAR_ARRAY:     astDecompile(node->son[0], out);
                                fprintf(out, " %s[%s];\n", node->son[1]->symbol->text, node->son[2]->symbol->text);
                                break;
        case AST_VAR_ARRAY_VALUE:
                                astDecompile(node->son[0], out);
                                fprintf(out, " %s[%s]: ", node->son[1]->symbol->text, node->son[2]->symbol->text); 
                                astDecompile(node->son[3], out);
                                break;
        case AST_FUNCTION_DEC:  astDecompile(node->son[0], out);
                                fprintf(out, " %s(", node->son[1]->symbol->text); 
                                astDecompile(node->son[2], out); 
                                fprintf(out, ")\n");
                                astDecompile(node->son[3], out); 
                                fprintf(out, "\n");
                                break;
        case AST_FUNCTION_CALL: fprintf(out, "%s(", node->son[0]->symbol->text);
                                astDecompile(node->son[1], out);
                                fprintf(out, ")");
                                break;
        case AST_PRINT_PARAM_STRING: 
                                fprintf(out, "%s", node->son[0]->symbol->text); 
                                if(node->son[1] != NULL) fprintf(out, ", ");
                                astDecompile(node->son[1], out);
                                break;
        case AST_PRINT_PARAM_EXP: 
                                astDecompile(node->son[0], out); 
                                if(node->son[1] != NULL) fprintf(out, ", ");
                                astDecompile(node->son[1], out);
                                break;
        case AST_FUNCTION_DEC_PARAM:
                                astDecompile(node->son[0], out); 
                                fprintf(out," %s",node->son[1]->symbol->text);
                                if(node->son[2] != NULL) fprintf(out, ", ");
                                astDecompile(node->son[2], out);
                                break;
        case AST_FUNCTION_CALL_PARAM: 
                                astDecompile(node->son[0], out);
                                if(node->son[1] != NULL) fprintf(out, ", ");
                                astDecompile(node->son[1], out);
                                break;
        default: fprintf(out, "\\ERROR!"); break; //Coloca no arquivo de saída um comentário sinalizando que algo deu errado
    }
}
       
