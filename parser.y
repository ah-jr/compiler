%{
  #include "semantic.h"
  #include "asm.h"

  int yyerror();
  int getLineNumber();
  int yylex();

  int g_errorCode = 0; 

  AST* astnode;
  int SyntaxErrors = 0;
%}

%union
{
  HASH_NODE *symbol;
  AST *ast;
}

%token KW_CHAR           
%token KW_INT            
%token KW_FLOAT          

%token KW_IF             
%token KW_THEN           
%token KW_ELSE           
%token<symbol>  KW_WHILE          
%token KW_GOTO           
%token KW_READ           
%token KW_PRINT          
%token KW_RETURN         

%token OPERATOR_LE       
%token OPERATOR_GE      
%token OPERATOR_L       
%token OPERATOR_G       
%token OPERATOR_EQ       
%token OPERATOR_DIF      

%token OPERATOR_SUM
%token OPERATOR_SUB
%token OPERATOR_MUL
%token OPERATOR_DIV

%token<symbol> TK_IDENTIFIER 
%token<symbol> LIT_INTEGER       
%token<symbol> LIT_CHAR          
%token<symbol> LIT_STRING      

%type<ast> declaration
%type<ast> commandList
%type<ast> command
%type<ast> goto
%type<ast> label
%type<ast> return 
%type<ast> while
%type<ast> content
%type<ast> print
%type<ast> printParam
%type<ast> variable
%type<ast> list
%type<ast> identifier
%type<ast> assignment
%type<ast> expression
%type<ast> functionCall
%type<ast> functionCallParam
%type<ast> function
%type<ast> functionParam
%type<ast> functionParamElements
%type<ast> body
%type<ast> if

%token TOKEN_ERROR  

%left OPERATOR_G OPERATOR_L OPERATOR_GE  OPERATOR_LE OPERATOR_DIF OPERATOR_EQ
%left OPERATOR_SUB OPERATOR_SUM
%left OPERATOR_MUL OPERATOR_DIV

%%

program: 
      declaration { astnode = $1; 
                    //astPrint(astnode, 0); 
                    check_and_set_declarations($1);
                    check_undeclared();
                    check_operands($1); 
                    check_assignments($1);
                    checkNativeFunctions($1);
                    check_and_optimize_loops($1, $1);

                    // Apenas gera código em caso de não haver erros de sintaxe
                    if (!SyntaxErrors)
                    {
                      TAC* code;
                      code = generateCode($1);
                      tacPrintBackwards(code);
                      code = tacReverse(code);
                      generateASM(code);
                    } 
                  }
    ;

declaration: 
      variable ';'    declaration  {$$ = astCreate(AST_DECLARATION, NULL, $1, $3, 0, 0);}
    | function        declaration  {$$ = astCreate(AST_DECLARATION, NULL, $1, $2, 0, 0);}
    |                              {$$ = NULL; }
    ;

commandList:
      command ';'     commandList  {$$ = astCreate(AST_COMMAND_LIST,       NULL, $1, $3,0,0);}
    | command {g_errorCode = 1;} error commandList  {$$ = 0;}   
    | label           commandList  {$$ = astCreate(AST_COMMAND_LIST_LABEL, NULL, $1, $2,0,0);}
    | body ';'        commandList  {$$ = astCreate(AST_COMMAND_LIST,       NULL, $1, $3,0,0);}
    | body    {g_errorCode = 1;} error commandList  {$$ = 0;}
    | command ';'                  {$$ = astCreate(AST_COMMAND_LIST,       NULL, $1, 0,0,0);}
    | label                        {$$ = astCreate(AST_COMMAND_LIST_LABEL, NULL, $1, 0,0,0);}
    | body ';'                     {$$ = astCreate(AST_COMMAND_LIST,       NULL, $1, 0,0,0);}
    ;

command:
      assignment  {$$ = $1;}    
    | print       {$$ = $1;}     
    | if          {$$ = $1;}    
    | while       {$$ = $1;}    
    | return      {$$ = $1;}    
    | goto        {$$ = $1;}
    |             {$$ = NULL;}
    ;   

goto:
    KW_GOTO TK_IDENTIFIER {$$ = astCreate(AST_GOTO, NULL, astCreate(AST_SYMBOL, $2, 0,0,0,0), 0,0,0);}
    ;

label:
    TK_IDENTIFIER ':'   {$$ = astCreate(AST_LABEL, NULL, astCreate(AST_SYMBOL, $1, 0,0,0,0), 0,0,0);}
    ;    


return :
    KW_RETURN expression {$$ = astCreate(AST_RETURN, NULL, $2, 0,0,0);}
    ;

while: 
    KW_WHILE expression content {$$ = astCreate(AST_WHILE, $1, $2,$3,0,0);}
    ;

content: 
      body     {$$ = $1;}
    | command  {$$ = $1;}
    ;

print: 
    KW_PRINT printParam  {$$ = astCreate(AST_PRINT, NULL, $2, 0,0,0);}
    ;

printParam: 
      LIT_STRING ',' printParam  {$$ = astCreate(AST_PRINT_PARAM_STRING, NULL, astCreate(AST_STRING, $1, 0,0,0,0), $3,0,0);}
    | expression ',' printParam  {$$ = astCreate(AST_PRINT_PARAM_EXP,    NULL, $1, $3,0,0);}
    | LIT_STRING                 {$$ = astCreate(AST_PRINT_PARAM_STRING, NULL, astCreate(AST_STRING, $1, 0,0,0,0), 0,0,0);} 
    | expression                 {$$ = astCreate(AST_PRINT_PARAM_EXP,    NULL, $1, 0,0,0);}
    ;

variable: 
      KW_CHAR   TK_IDENTIFIER ':' LIT_INTEGER                           {$$ = astCreate(AST_VAR,       $2, astCreate(AST_CHAR, NULL, 0,0,0,0),  astCreate(AST_SYMBOL, $2, 0,0,0,0), astCreate(AST_SYMBOL, $4,0,0,0,0), 0);}
    | KW_INT    TK_IDENTIFIER ':' LIT_INTEGER                           {$$ = astCreate(AST_VAR,       $2, astCreate(AST_INT, NULL, 0,0,0,0),   astCreate(AST_SYMBOL, $2, 0,0,0,0), astCreate(AST_SYMBOL, $4,0,0,0,0), 0);}
    | KW_CHAR   TK_IDENTIFIER ':' LIT_CHAR                              {$$ = astCreate(AST_VAR,       $2, astCreate(AST_CHAR, NULL, 0,0,0,0),  astCreate(AST_SYMBOL, $2, 0,0,0,0), astCreate(AST_SYMBOL, $4,0,0,0,0), 0);}
    | KW_INT    TK_IDENTIFIER ':' LIT_CHAR                              {$$ = astCreate(AST_VAR,       $2, astCreate(AST_INT, NULL, 0,0,0,0),   astCreate(AST_SYMBOL, $2, 0,0,0,0), astCreate(AST_SYMBOL, $4,0,0,0,0), 0);}
    | KW_FLOAT  TK_IDENTIFIER ':' LIT_INTEGER OPERATOR_DIV LIT_INTEGER  {$$ = astCreate(AST_VAR_FLOAT, $2, astCreate(AST_FLOAT, NULL, 0,0,0,0), astCreate(AST_SYMBOL, $2, 0,0,0,0), astCreate(AST_SYMBOL, $4,0,0,0,0), astCreate(AST_SYMBOL, $6,0,0,0,0));}
    | KW_CHAR   TK_IDENTIFIER '[' LIT_INTEGER ']'                       {$$ = astCreate(AST_VAR_ARRAY, $2, astCreate(AST_CHAR, NULL, 0,0,0,0),  astCreate(AST_SYMBOL, $2,0,0,0,0),  astCreate(AST_SYMBOL, $4,0,0,0,0), 0);}
    | KW_INT    TK_IDENTIFIER '[' LIT_INTEGER ']'                       {$$ = astCreate(AST_VAR_ARRAY, $2, astCreate(AST_INT, NULL, 0,0,0,0),   astCreate(AST_SYMBOL, $2,0,0,0,0),  astCreate(AST_SYMBOL, $4,0,0,0,0), 0);}
    | KW_FLOAT  TK_IDENTIFIER '[' LIT_INTEGER ']'                       {$$ = astCreate(AST_VAR_ARRAY, $2, astCreate(AST_FLOAT, NULL, 0,0,0,0),       astCreate(AST_SYMBOL, $2,0,0,0,0),  astCreate(AST_SYMBOL, $4,0,0,0,0), 0);}
    | KW_CHAR   TK_IDENTIFIER '[' LIT_INTEGER ']' ':' list              {$$ = astCreate(AST_VAR_ARRAY_VALUE, $2, astCreate(AST_CHAR, NULL, 0,0,0,0),  astCreate(AST_SYMBOL, $2,0,0,0,0),  astCreate(AST_SYMBOL, $4,0,0,0,0), $7);}
    | KW_INT    TK_IDENTIFIER '[' LIT_INTEGER ']' ':' list              {$$ = astCreate(AST_VAR_ARRAY_VALUE, $2, astCreate(AST_INT, NULL, 0,0,0,0),   astCreate(AST_SYMBOL, $2,0,0,0,0),  astCreate(AST_SYMBOL, $4,0,0,0,0), $7);}
    | KW_FLOAT  TK_IDENTIFIER '[' LIT_INTEGER ']' ':' list              {$$ = astCreate(AST_VAR_ARRAY_VALUE, $2, astCreate(AST_FLOAT, NULL, 0,0,0,0), astCreate(AST_SYMBOL, $2,0,0,0,0),  astCreate(AST_SYMBOL, $4,0,0,0,0), $7);}
    | KW_CHAR   TK_IDENTIFIER error LIT_INTEGER                         {$$ = 0; fprintf(stderr, "Missing \":\"\n");}
    | KW_INT    TK_IDENTIFIER error LIT_INTEGER                         {$$ = 0; fprintf(stderr, "Missing \":\"\n");}
    | KW_CHAR   TK_IDENTIFIER error LIT_CHAR                            {$$ = 0; fprintf(stderr, "Missing \":\"\n");}
    | KW_CHAR   TK_IDENTIFIER '[' error ']'                             {$$ = 0; fprintf(stderr, "Expecting array size\n");}
    | KW_INT    TK_IDENTIFIER '[' error ']'                             {$$ = 0; fprintf(stderr, "Expecting array size\n");}
    | KW_FLOAT  TK_IDENTIFIER '[' error ']'                             {$$ = 0; fprintf(stderr, "Expecting array size\n");}

    ;

list: 
      LIT_INTEGER list  {$$ = astCreate(AST_LIST, NULL, astCreate(AST_SYMBOL, $1,0,0,0,0), $2, 0, 0);}
    | LIT_CHAR    list  {$$ = astCreate(AST_LIST, NULL, astCreate(AST_SYMBOL, $1,0,0,0,0), $2, 0, 0);}
    | LIT_INTEGER       {$$ = astCreate(AST_LIST, NULL, astCreate(AST_SYMBOL, $1,0,0,0,0), 0, 0, 0);}  
    | LIT_CHAR          {$$ = astCreate(AST_LIST, NULL, astCreate(AST_SYMBOL, $1,0,0,0,0), 0, 0, 0);}
    ;

identifier: 
      TK_IDENTIFIER                    {$$ = astCreate(AST_SYMBOL, $1,0,0,0,0);}
    | TK_IDENTIFIER '[' expression ']' {$$ = astCreate(AST_ARRAY_ACCESS, $1, astCreate(AST_SYMBOL, $1,0,0,0,0), $3,0,0);} 
    ;

assignment: 
      TK_IDENTIFIER                    '=' expression  {$$ = astCreate(AST_ASSIGN, $1, astCreate(AST_SYMBOL, $1,0,0,0,0), $3,0,0);}
    | TK_IDENTIFIER '[' expression ']' '=' expression  {$$ = astCreate(AST_ARRAY_ASSIGN, $1, astCreate(AST_SYMBOL, $1,0,0,0,0), $3, $6,0);}
    | TK_IDENTIFIER '=' error                          {$$ = 0; fprintf(stderr, "Wrong assignment\n");}
    ;

expression:  
      expression OPERATOR_SUM expression  {$$ = astCreate(AST_ADD, 0, $1, $3,0,0);}
    | expression OPERATOR_SUB expression  {$$ = astCreate(AST_SUB, 0, $1, $3,0,0);}
    | expression OPERATOR_MUL expression  {$$ = astCreate(AST_MUL, 0, $1, $3,0,0);}
    | expression OPERATOR_DIV expression  {$$ = astCreate(AST_DIV, 0, $1, $3,0,0);}
    | expression OPERATOR_L   expression  {$$ = astCreate(AST_L,   0, $1, $3,0,0);}
    | expression OPERATOR_G   expression  {$$ = astCreate(AST_G,   0, $1, $3,0,0);}
    | expression OPERATOR_LE  expression  {$$ = astCreate(AST_LE,  0, $1, $3,0,0);}
    | expression OPERATOR_GE  expression  {$$ = astCreate(AST_GE,  0, $1, $3,0,0);}
    | expression OPERATOR_EQ  expression  {$$ = astCreate(AST_EQ,  0, $1, $3,0,0);}
    | expression OPERATOR_DIF expression  {$$ = astCreate(AST_DIF, 0, $1, $3,0,0);}
    | '(' expression ')'                  {$$ = astCreate(AST_CAPSULE, NULL, $2, 0, 0, 0);}
    | '(' error ')'                             {$$ = 0; fprintf(stderr, "Missing expression inside \"()\"\n");}
    | identifier                          {$$ = $1;}
    | LIT_INTEGER                         {$$ = astCreate(AST_SYMBOL, $1, 0,0,0,0); }
    | LIT_CHAR                            {$$ = astCreate(AST_SYMBOL, $1, 0,0,0,0); }
    | functionCall                        {$$ = $1;}
    | KW_READ                             {$$ = astCreate(AST_READ, NULL, 0,0,0,0); }
    ;

functionCall:
      TK_IDENTIFIER '(' functionCallParam ')'  {$$ = astCreate(AST_FUNCTION_CALL, $1, astCreate(AST_SYMBOL, $1,0,0,0,0), $3,0,0);}
    | TK_IDENTIFIER '(' ')'                    {$$ = astCreate(AST_FUNCTION_CALL, $1, astCreate(AST_SYMBOL, $1,0,0,0,0), 0,0,0);}
    | TK_IDENTIFIER error ')'                  {$$ = 0; fprintf(stderr, "Missing \")\" in function call\n");}
    | TK_IDENTIFIER error                      {$$ = 0; fprintf(stderr, "Missing \"()\" in function call\n");}
    ;

functionCallParam:
      expression ',' functionCallParam {$$ = astCreate(AST_FUNCTION_CALL_PARAM, NULL, $1,$3,0,0);}
    | expression                       {$$ = astCreate(AST_FUNCTION_CALL_PARAM, NULL, $1,0,0,0);}
    ;

function:  
      KW_CHAR  TK_IDENTIFIER functionParam content {$$ = astCreate(AST_FUNCTION_DEC, $2, astCreate(AST_CHAR, NULL, 0,0,0,0),  astCreate(AST_SYMBOL, $2, 0,0,0,0), $3, $4);}
    | KW_INT   TK_IDENTIFIER functionParam content {$$ = astCreate(AST_FUNCTION_DEC, $2, astCreate(AST_INT, NULL, 0,0,0,0),   astCreate(AST_SYMBOL, $2, 0,0,0,0), $3, $4);}
    | KW_FLOAT TK_IDENTIFIER functionParam content {$$ = astCreate(AST_FUNCTION_DEC, $2, astCreate(AST_FLOAT, NULL, 0,0,0,0), astCreate(AST_SYMBOL, $2, 0,0,0,0), $3, $4);}
    ;

functionParam: 
      '(' ')'                       {$$ = NULL;}
    | '(' functionParamElements ')' {$$ = $2;}
    | error                         {$$ = 0; fprintf(stderr, "Missing \"()\" in function declaration\n");}
    ;

functionParamElements:
      KW_CHAR  TK_IDENTIFIER ',' functionParamElements {$$ = astCreate(AST_FUNCTION_DEC_PARAM, $2, astCreate(AST_CHAR, NULL, 0,0,0,0),  astCreate(AST_SYMBOL, $2, 0,0,0,0), $4, 0);}
    | KW_INT   TK_IDENTIFIER ',' functionParamElements {$$ = astCreate(AST_FUNCTION_DEC_PARAM, $2, astCreate(AST_INT, NULL, 0,0,0,0),   astCreate(AST_SYMBOL, $2, 0,0,0,0), $4, 0);}
    | KW_FLOAT TK_IDENTIFIER ',' functionParamElements {$$ = astCreate(AST_FUNCTION_DEC_PARAM, $2, astCreate(AST_FLOAT, NULL, 0,0,0,0), astCreate(AST_SYMBOL, $2, 0,0,0,0), $4, 0);}
    | KW_CHAR  TK_IDENTIFIER                           {$$ = astCreate(AST_FUNCTION_DEC_PARAM, $2, astCreate(AST_CHAR, NULL, 0,0,0,0),  astCreate(AST_SYMBOL, $2, 0,0,0,0), 0, 0);}
    | KW_INT   TK_IDENTIFIER                           {$$ = astCreate(AST_FUNCTION_DEC_PARAM, $2, astCreate(AST_INT, NULL, 0,0,0,0),   astCreate(AST_SYMBOL, $2, 0,0,0,0), 0, 0);}
    | KW_FLOAT TK_IDENTIFIER                           {$$ = astCreate(AST_FUNCTION_DEC_PARAM, $2, astCreate(AST_FLOAT, NULL, 0,0,0,0), astCreate(AST_SYMBOL, $2, 0,0,0,0), 0, 0);}
    ;

body: 
      '{' '}'               { $$ = astCreate(AST_BODY, NULL, 0, 0, 0, 0);}
    | '{' commandList '}'   { $$ = astCreate(AST_BODY, NULL, $2, 0, 0, 0);}
    | '{' error             { $$ = 0; fprintf(stderr, "Missing \"}\" at the end of body\n");}
    | '{' commandList error { $$ = 0; fprintf(stderr, "Missing \"}\" at the end of body\n");}
    ;

if: 
      KW_IF expression KW_THEN content                  {$$ = astCreate(AST_IF, NULL, $2, $4,0,0);}
    | KW_IF expression KW_THEN content KW_ELSE content  {$$ = astCreate(AST_IF, NULL, $2, $4, $6,0);}
    ;

%%

int yyerror()
{
    fprintf(stderr, "\nSyntax Error: Line $%d\n", getLineNumber());

    switch(g_errorCode){
      case 1:
        fprintf(stderr, "Expecting \";\"\n");
        break;
    }

    g_errorCode = 0;
    SyntaxErrors++;
    return 1;
} 
