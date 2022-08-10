%{

#include "ast.h"
#include "y.tab.h"

int lineCount = 1;
int running = 1;

int yyerror();

%}

%x COMMENT

%%

int                        { return KW_INT; }
float                      { return KW_FLOAT; }
char                       { return KW_CHAR; }

if                         { return KW_IF; }
then                       { return KW_THEN; }
else                       { return KW_ELSE; }

print                      { return KW_PRINT; }
return                     { return KW_RETURN; }
while                      { yylval.symbol = hashInsert(yytext, SYMBOL_SPECIAL_WHILE); // Insere na hash, para podermos otimizar os laços
                             return KW_WHILE; }
goto                       { return KW_GOTO; }
read                       { return KW_READ; }

"=="                       { return OPERATOR_EQ; }
"!="                       { return OPERATOR_DIF; }
"<="                       { return OPERATOR_LE; }
">="                       { return OPERATOR_GE; }
"<"                        { return OPERATOR_L; }
">"                        { return OPERATOR_G; }

"\+"                       { return OPERATOR_SUM; }
"\-"                       { return OPERATOR_SUB; }
"/"                        { return OPERATOR_DIV; }
"*"                        { return OPERATOR_MUL; }

[,;:\(\)\[\]\{\}=]         { return yytext[0]; }

[0-9]+                     { yylval.symbol = hashInsert(yytext, SYMBOL_INTEGER);
                             return LIT_INTEGER; }

[-_a-z]+                   { yylval.symbol = hashInsert(yytext, SYMBOL_IDENTIFIER); 
                             return TK_IDENTIFIER; }

\'.\'                      { yylval.symbol = hashInsert(yytext, SYMBOL_CHAR); 
                             return LIT_CHAR; }

\"([^"\\\n]|\\.)*\"        { yylval.symbol = hashInsert(yytext, SYMBOL_STRING); 
                             return LIT_STRING; }

"\n"                       { lineCount++; }

"\\*"                      { BEGIN(COMMENT); }
<COMMENT>"*\\"             { BEGIN(INITIAL); }
<COMMENT>"\n"              { lineCount++; }

[ \t]
"\\\\".*                   
<COMMENT>.                 

.                          { return TOKEN_ERROR; }

%%

void initMe(void);
int getLineNumber(void);
int isRunning(void);


void initMe(void)
{
    hashInit();
}

int yywrap() 
{
    running = 0;
    return 1;
}

int isRunning(void)
{
    return running;
}

int getLineNumber(void)
{
    return lineCount;
}