#include <stdio.h>
#include "semantic.h"

extern FILE *yyin;
int yyparse();
extern AST *astnode;
extern int SyntaxErrors;
 
int main(int argc, char** argv) 
{
    FILE *yyout;

    if(argc == 3)
    {
        if((yyin = fopen(argv[1],"r")) != NULL)
        {
            yyparse();
        }
        else
        {
            printf("Não foi possível abrir o arquivo de entrada\n");
            exit(2);
        }

        if ((yyout = fopen(argv[2],"w")) == NULL)
        {
            printf("Não foi possível abrir o arquivo de saida\n");
            exit(2);
        }
    }
    else  
    {
        printf("Especifique o arquivo de entrada: etapa1.exe in.txt out.txt\n");
        exit(1);
    }

    astDecompile(astnode, yyout);   
    fclose(yyout);

    //hashPrint();

    if (SemanticErrors)
    {
        fprintf(stderr, "Compilation was unsuccessful due to semantic errors!\n");  
        exit(4); 
    }
    else if (SyntaxErrors)
    {
        fprintf(stderr, "Compilation was unsuccessful due to syntax errors!\n");  
        exit(3); 
    }
    else    
    {
        fprintf(stderr, "Compilation was successful!\n");
        exit(0); 
    }

}