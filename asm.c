#include "asm.h"

void freeArgList(PRINT_ARG_LIST* head)
{
   PRINT_ARG_LIST* tmp;

   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }
}

void handleBooleanExp(TAC* tac, FILE* fout, const char* op,  const char* title){
    fprintf(fout, "## %s\n", title);

    if (tac->op1->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%edx\n", tac->op1->text);
    else                                  fprintf(fout, "    movl	_%s(%%rip), %%edx\n", tac->op1->text);

    if (tac->op2->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n", tac->op2->text);
    else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op2->text);

    fprintf(fout, "\tcmpl\t%%eax, %%edx\n");
    fprintf(fout, "\t%s\t%%al\n", op);
    fprintf(fout,"\tmovzbl\t%%al, %%eax\n");

    if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl   %%eax, $%s\n\n", tac->res->text);
    else                                  fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);
}

TAC* tacReverse(TAC *tac)
{
    TAC* t = tac; 
    for (t=tac; t->prev; t = t->prev)
        t->prev->next = t;

    return t;
}

void generateASM(TAC* first)
{
    FILE *fout;
    fout = fopen("out.s", "w");
    char *strAux; 
    int strIndex = 0;
    int param_dec_counter = 0;
    int param_call_counter = 0;

    PRINT_ARG_LIST *pt_args = NULL;
    PRINT_ARG_LIST *pt_args_last = NULL;
    PRINT_ARG_LIST *pt_args_aux = NULL;
   
    fprintf(fout, 
    "## FIXED INIT\n"
	".text\n"
	".section	.rodata\n"
	".text\n"
    ".PrintIntStr:\n"
    "	.string	\"%%d\\n\"\n"
    ".PrintStringStr:\n"
    "	.string	\"%%s\\n\"\n\n"
    ".ReadIntStr:\n"
    "	.string	\"%%d\"\n\n"
    "## FIX MAIN NAME\n"
    ".globl	main\n"
    "main: jmp _reserved_name_function_main\n\n"
    );

    for (TAC* tac = first; tac; tac = tac->next)
    {
        switch(tac->type)
        {
            case TAC_BEGINFUN:
                param_dec_counter = 1;

                strAux = tac->res->text;
                fprintf(fout, 
                "## TAC_BEGINFUN\n"
                ".globl	%s\n"
	            ".type	%s, @function\n"
                "%s:\n", strAux, strAux, strAux
                );
                break;

            case TAC_ENDFUN:
                fprintf(fout, 
                "## TAC_ENDFUN\n"
	            "    popq	%%rbp\n"
	            "    ret\n\n"
                );

                break;

            case TAC_AFTER_PARAM:
                fprintf(fout, 
                "## TAC_AFTER_PARAM\n"
                "    pushq	%%rbp\n"
                "    movq	%%rsp, %%rbp\n\n"
                );
                break;  

            case TAC_PRINTARG:
                if (!pt_args) 
                {
                    pt_args = (PRINT_ARG_LIST*) malloc(sizeof(PRINT_ARG_LIST));
                    pt_args->next = NULL;
                    pt_args->text = tac->res->text;
                    switch(tac->res->type){
                        case SYMBOL_STRING: pt_args->type = PT_STRING; break;
                        case SYMBOL_INTEGER: pt_args->type = PT_INT; break;
                        case SYMBOL_VARIABLE: pt_args->type = PT_VAR; break;    
                    }
                    pt_args->index = strIndex;

                    pt_args_last = pt_args;
                }
                else if (!pt_args_last) 
                {
                    pt_args_aux = pt_args;
                    while(pt_args_aux->next) pt_args_aux = pt_args_aux->next;

                    pt_args_last = (PRINT_ARG_LIST*) malloc(sizeof(PRINT_ARG_LIST));
                    pt_args_last->next = NULL;
                    pt_args_last->text = tac->res->text;
                    switch(tac->res->type){
                        case SYMBOL_STRING: pt_args_last->type = PT_STRING; break;
                        case SYMBOL_INTEGER: pt_args_last->type = PT_INT; break;
                        case SYMBOL_VARIABLE: pt_args_last->type = PT_VAR; break;    
                    }
                    pt_args_last->index = strIndex;

                    pt_args_aux->next = pt_args_last;
                }
                else
                {
                    pt_args_aux = pt_args_last;
                    while(pt_args_aux->next) pt_args_aux = pt_args_aux->next;

                    pt_args_aux->next = (PRINT_ARG_LIST*) malloc(sizeof(PRINT_ARG_LIST));
                    pt_args_aux->next->next  = NULL;
                    pt_args_aux->next->text  = tac->res->text;
                    switch(tac->res->type){
                        case SYMBOL_STRING: pt_args_aux->next->type = PT_STRING; break;
                        case SYMBOL_INTEGER: pt_args_aux->next->type = PT_INT; break;
                        case SYMBOL_VARIABLE: pt_args_aux->next->type = PT_VAR; break;    
                    }
                    pt_args_aux->next->index = strIndex;
                }

                strIndex++;
                break;

            case TAC_PRINT:
                pt_args_aux = pt_args_last;

                while(pt_args_aux){
                    if (pt_args_aux->type == PT_STRING){
                        fprintf(fout, 
                        "## TAC_PRINT\n"
                        "    leaq	_reserved_name_string_%d(%%rip), %%rsi\n"
                        "    leaq	.PrintStringStr(%%rip), %%rdi\n"
                        "    call	printf@PLT\n\n", pt_args_aux->index
                        );
                    }
                    else if (pt_args_aux->type == PT_INT){
                        fprintf(fout, 
                        "## TAC_PRINT\n"
                        "    movl	_reserved_name_string_%d(%%rip), %%esi\n"
                        "    leaq	.PrintIntStr(%%rip), %%rdi\n"
                        "    call	printf@PLT\n\n", pt_args_aux->index
                        );                        
                    }
                    else if (pt_args_aux->type == PT_VAR){
                        fprintf(fout, 
                        "## TAC_PRINT\n"
                        "    movl	_%s(%%rip), %%esi\n"
                        "    leaq	.PrintIntStr(%%rip), %%rdi\n"
                        "    call	printf@PLT\n\n", pt_args_aux->text
                        );                        
                    }

                    pt_args_aux = pt_args_aux->next;
                }

                pt_args_last = NULL;
                break;       

            case TAC_COPY:
                fprintf(fout, "## TAC_COPY\n");
                if (tac->op1->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n", tac->op1->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op1->text);

                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl   %%eax, $%s\n\n", tac->res->text);
                else                                  fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);
                
                break;

            case TAC_ADD:
                fprintf(fout, "## TAC_ADD\n");
                if (tac->op1->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n", tac->op1->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op1->text);

                if (tac->op2->type == SYMBOL_INTEGER) fprintf(fout, "    addl	$%s, %%eax\n", tac->op2->text);
                else                                  fprintf(fout, "    addl	_%s(%%rip), %%eax\n", tac->op2->text);
                
                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl   %%eax, $%s\n\n", tac->res->text);
                else                                  fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);

                break;

            case TAC_SUB:
                fprintf(fout, "## TAC_SUB\n");
                if (tac->op1->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n", tac->op1->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op1->text);

                if (tac->op2->type == SYMBOL_INTEGER) fprintf(fout, "    subl	$%s, %%eax\n", tac->op2->text);
                else                                  fprintf(fout, "    subl	_%s(%%rip), %%eax\n", tac->op2->text);
                
                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl   %%eax, $%s\n\n", tac->res->text);
                else                                  fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);

                break;

            case TAC_MUL:
                fprintf(fout, "## TAC_MUL\n");
                if (tac->op1->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n", tac->op1->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op1->text);

                if (tac->op2->type == SYMBOL_INTEGER) fprintf(fout, "    imull	$%s, %%eax\n", tac->op2->text);
                else                                  fprintf(fout, "    imull	_%s(%%rip), %%eax\n", tac->op2->text);
                
                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl   %%eax, $%s\n\n", tac->res->text);
                else                                  fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);

                break;

            case TAC_DIV:
                fprintf(fout, "## TAC_DIV\n");
                if (tac->op1->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n", tac->op1->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op1->text);

                if (tac->op2->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%ecx\n", tac->op2->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%ecx\n", tac->op2->text);

                fprintf(fout, "\tcltd\n");
                fprintf(fout, "\tidivl\t%%ecx\n");
               
                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl   %%eax, $%s\n\n", tac->res->text);
                else                                  fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);

                break;

            case TAC_EQ: handleBooleanExp(tac, fout, "sete", "TAC_EQ"); break;
            case TAC_DIF:handleBooleanExp(tac, fout, "setne", "TAC_DIF"); break;
            case TAC_GE: handleBooleanExp(tac, fout, "setge", "TAC_GE"); break;
            case TAC_G:  handleBooleanExp(tac, fout, "setg", "TAC_G"); break;
            case TAC_LE: handleBooleanExp(tac, fout, "setle", "TAC_LE"); break;
            case TAC_L:  handleBooleanExp(tac, fout, "setl", "TAC_L"); break;

            case TAC_JF:  
                fprintf(fout, "## TAC_JF\n");
                fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op1->text);           
                fprintf(fout, "    testl\t%%eax, %%eax\n");
                fprintf(fout, "    je	_%s\n\n", tac->res->text);  
                break;   

            case TAC_JUMP:
                fprintf(fout, "## TAC_JUMP\n");
                fprintf(fout, "    jmp _%s\n\n", tac->res->text);
                break;

            case TAC_LABEL:
                fprintf(fout, "## TAC_LABEL\n");
                fprintf(fout, "_%s:\n\n", tac->res->text);
                break;

            case TAC_READ:
                fprintf(fout, "## TAC_READ\n");

                fprintf(fout, "    leaq    _%s(%%rip), %%rsi\n", tac->res->text);
                fprintf(fout, "    leaq    .ReadIntStr(%%rip), %%rdi\n");
                fprintf(fout, "    call    __isoc99_scanf@PLT\n\n");
                break;

            case TAC_RET:
                fprintf(fout, "## TAC_RET\n");
                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n", tac->res->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->res->text);
                
                fprintf(fout, "    movl   %%eax, reserved_return_register(%%rip)\n\n");
                
                break;

            case TAC_ARG:
                param_call_counter++;

                fprintf(fout, "## TAC_ARG\n");   
                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    pushq	$%s\n", tac->res->text);
                else                                  fprintf(fout, "    pushq  _%s(%%rip)\n\n", tac->res->text);  

                break; 

            case TAC_CALL:
                fprintf(fout, "## TAC_CALL\n");
                fprintf(fout, "    call _reserved_name_function_%s\n", tac->op1->text); //jump para a função
                 for (int i = 0; i<param_call_counter; i++)
                     fprintf(fout, "    popq %%r12\n"); //limpar pilha

                fprintf(fout, "\n    movl	reserved_return_register(%%rip), %%eax\n");      //copia o retorno salvo

                if (tac->res->type == SYMBOL_INTEGER) fprintf(fout, "    movl   %%eax, $%s\n\n", tac->res->text);
                else                                  fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);

                param_call_counter = 0;

                break;

            case TAC_PARAM:
                fprintf(fout, "## TAC_PARAM\n");
                fprintf(fout, "    movq %d(%%rsp), %%rax\n", param_dec_counter * 8);
                fprintf(fout, "    movq %%rax, _%s(%%rip)\n\n", tac->res->text);          

                param_dec_counter++; 

                break;

            case TAC_ARRAY:
                fprintf(fout, "## TAC_ARRAY\n");
                fprintf(fout, "    movl	_%s_%s(%%rip), %%eax\n", tac->op1->text, tac->op2->text);
                fprintf(fout, "    movl   %%eax, _%s(%%rip)\n\n", tac->res->text);
               
                break;

            case TAC_ARRAY_ASSIGN:
                fprintf(fout, "## TAC_ARRAY_ASSIGN\n");

                if (tac->op2->type == SYMBOL_INTEGER) fprintf(fout, "    movl	$%s, %%eax\n",        tac->op2->text);
                else                                  fprintf(fout, "    movl	_%s(%%rip), %%eax\n", tac->op2->text); 
                
                fprintf(fout, "    movl %%eax, _%s_%s(%%rip)\n\n", tac->res->text, tac->op1->text);
               
                break;   
        }
    }

    fprintf(fout, "## DATA SECTION \n");
    fprintf(fout, ".section .data\n");
    printASM(fout);

    //Printar as strings no final
    pt_args_aux = pt_args;

    while(pt_args_aux){
        if (pt_args_aux->type != PT_VAR) fprintf(fout, "_reserved_name_string_%d: ", pt_args_aux->index);

        if (pt_args_aux->type == PT_STRING) fprintf(fout, ".string %s\n", pt_args_aux->text);
        else if (pt_args_aux->type == PT_INT) fprintf(fout, ".long %s\n", pt_args_aux->text);

        pt_args_aux = pt_args_aux->next; 
    }

    //Printar o valor especial de return, que deve conter o retorno da última função executada
    fprintf(fout, "reserved_return_register: ");
    fprintf(fout, ".long 10\n");

    freeArgList(pt_args);
    fclose(fout);
}