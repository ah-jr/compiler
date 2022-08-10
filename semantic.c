#include "semantic.h"
#include "stdlib.h"

int SemanticErrors = 0;


void check_assignments(AST *node)
{
    if (node == 0){
        return;
    }

    if (node->type == AST_ASSIGN)
    {
        if(node->son[0] && node->son[0]->symbol)
        {
            if(node->son[0]->type == AST_SYMBOL)
            {
                if (!(node->son[1] && check_valid_arithmetical_operand(node->son[1])))
                {
                    fprintf(stderr, "Semantic ERROR: invalid assignment for identifier %s\n", node->son[0]->symbol->text);
                    SemanticErrors++;
                }

                if (node->son[0]->symbol->type == SYMBOL_FUNCTION)
                {
                    fprintf(stderr, "Semantic ERROR: Can't Assign to FUNCTION %s\n", node->son[0]->symbol->text);
                    SemanticErrors++;
                }
                else if (node->son[0]->symbol->auxnum != 0)
                {
                    fprintf(stderr, "Semantic ERROR: Can't Assign to ARRAY %s\n", node->son[0]->symbol->text);
                    SemanticErrors++;
                }
            }

            if(node->son[0]->type == AST_ARRAY_ACCESS) 
            {
                if (!(expressionIsInteger(node->son[0]->son[1])) && (node->son[0]->symbol->auxnum != 0))
                {
                    fprintf(stderr, "Semantic ERROR: invalid index for array %s\n", node->son[0]->symbol->text);
                    SemanticErrors++;
                } 
                if (!(node->son[1] && check_valid_arithmetical_operand(node->son[1])))
                {
                    fprintf(stderr, "Semantic ERROR: invalid assignment for identifier %s\n", node->son[0]->symbol->text);
                    SemanticErrors++;
                }
                if (node->son[0]->symbol->auxnum == 0)
                {
                    fprintf(stderr, "Semantic ERROR: %s is not an ARRAY\n", node->son[0]->symbol->text);
                    SemanticErrors++;
                }
            }
        }
    }

    for (int i = 0; i< MAX_SONS; i++){
        check_assignments(node->son[i]);
    }
}

void check_and_set_declarations(AST *node)
{
    if (node == 0){
        return;
    }

    switch (node->type)
    {
        case AST_VAR: 
            setNodeType(node, SYMBOL_VARIABLE);
            break; 
    
        case AST_VAR_FLOAT: 
            setNodeType(node, SYMBOL_VARIABLE);
            break;     

        case AST_VAR_ARRAY: 
            setNodeType(node, SYMBOL_VARIABLE);
            break;     

        case AST_VAR_ARRAY_VALUE: 
            setNodeType(node, SYMBOL_VARIABLE);
            break;       

        case AST_FUNCTION_DEC:  
            setNodeType(node, SYMBOL_FUNCTION);
            break; 

        case AST_FUNCTION_DEC_PARAM:
            setNodeType(node, SYMBOL_VARIABLE);
    }

    for (int i = 0; i< MAX_SONS; i++){
        check_and_set_declarations(node->son[i]);
    }
}

void setNodeType(AST *node, int symbol)
{
    AST *aux;

    if (node->symbol) {
        if (node->symbol->type != SYMBOL_IDENTIFIER) {
            if (node->symbol->text)
                fprintf(stderr, "Semantic ERROR: Identifier %s redeclared\n", node->symbol->text);
            SemanticErrors++;
        } 

        if(node->son[0]) {
            switch(node->son[0]->type)
            {
                case AST_INT:
                    node->symbol->datatype = DATATYPE_INT;
                    if (node->son[2] && node->son[2]->symbol)
                        node->symbol->init = atoi(node->son[2]->symbol->text);                
                    break;
                case AST_FLOAT:
                    node->symbol->datatype = DATATYPE_FLOAT;
                    break;
                case AST_CHAR:
                    node->symbol->datatype = DATATYPE_CHAR;
                    break;
            }
        }

        if ((node->type == AST_VAR_ARRAY) || (node->type == AST_VAR_ARRAY_VALUE)) 
        {
            node->symbol->auxnum = atoi(node->son[2]->symbol->text);
        }

        if (node->type == AST_VAR_ARRAY_VALUE)
        {
            int size = 0;
            if (node->son[3])
            {
                AST* auxnode;
                auxnode = node->son[3];

                while (auxnode)
                {
                    size++;
                    auxnode = auxnode->son[1];
                }
            }
            if (size != node->symbol->auxnum)
            {
                fprintf(stderr, "Semantic ERROR: Incorrect number of values in %s's declaration\n", node->son[1]->symbol->text);
                SemanticErrors++;
            }

            if ((node->symbol->auxnum > 0) && node->son[3])
            {
                int index = 0;
                aux = node->son[3];

                while (aux->son[1]){
                    node->symbol->inits[index++] = atoi(aux->son[0]->symbol->text);
                    aux = aux->son[1];
                }
                node->symbol->inits[index++] = atoi(aux->son[0]->symbol->text);
            }        
        }

        if (node->type == AST_FUNCTION_DEC)
        {
            if (node->son[2])
            {
                node->symbol->auxnum = 1;

                AST* auxnode;
                auxnode = node->son[2];

                while (auxnode)
                {
                    node->symbol->auxnum++;
                    auxnode = auxnode->son[2];
                }
            }
        }

        node->symbol->type = symbol;
    }
}


int check_valid_arithmetical_operand(AST *node)
{
    if (node->type == AST_CAPSULE) return check_valid_arithmetical_operand(node->son[0]); 

    if (node->type == AST_ADD || 
        node->type == AST_SUB || 
        node->type == AST_MUL || 
        node->type == AST_DIV)
    {
        return check_valid_arithmetical_operand(node->son[0]) * check_valid_arithmetical_operand(node->son[1]);  
    } 

    if (
        node->type == AST_READ   ||
        (
            (
                ((node->type == AST_SYMBOL)       && (node->symbol->auxnum == 0)) 
                ||
                ((node->type == AST_ARRAY_ACCESS) && (expressionIsInteger(node->son[1])) && (node->symbol->auxnum != 0)) 
            )
            &&
            ((node->symbol->datatype == DATATYPE_INT)   || 
             (node->symbol->datatype == DATATYPE_CHAR)  ||
             (node->symbol->datatype == DATATYPE_FLOAT) ||
             (node->symbol->type == SYMBOL_INTEGER)     || 
             (node->symbol->type == SYMBOL_CHAR)        ||
             (node->symbol->type == SYMBOL_FLOAT))
        )
        ||
        ((node->type == AST_FUNCTION_CALL) && (node->symbol) && (getParamCount(node) == node->symbol->auxnum) && check_function_operands(node) &&
            ((node->symbol->datatype == DATATYPE_INT)   || 
             (node->symbol->datatype == DATATYPE_CHAR)  ||
             (node->symbol->datatype == DATATYPE_FLOAT))
        )
    ) return 1;

    return 0;
}

int check_function_operands(AST *node)
{   
    if (node->type == AST_FUNCTION_CALL){
        AST* auxnode;
        auxnode = node->son[1];

        while (auxnode) {
            if (!check_valid_arithmetical_operand(auxnode->son[0]))
                return 0; 
            auxnode = auxnode->son[1];
        }
        return 1;
    } 
    return 0;
}

int check_valid_boolean_operand(AST *node)
{
    if (node->type == AST_CAPSULE) return check_valid_boolean_operand(node->son[0]);

    if (
        node->type == AST_L || 
        node->type == AST_G || 
        node->type == AST_LE || 
        node->type == AST_GE)
    {
        return check_valid_arithmetical_operand(node->son[0]) * check_valid_arithmetical_operand(node->son[1]);    
    }

    if (  
        node->type == AST_EQ || 
        node->type == AST_DIF)
    {
        return check_valid_boolean_operand(node->son[0]) * check_valid_boolean_operand(node->son[1]);          
    }

    if (
        node->type == AST_READ   ||
        (
            (
                ((node->type == AST_SYMBOL)       && (node->symbol->auxnum == 0)) 
                ||
                ((node->type == AST_ARRAY_ACCESS) && (expressionIsInteger(node->son[1])) && (node->symbol->auxnum != 0)) 
            )
            &&
            ((node->symbol->datatype == DATATYPE_INT)   || 
             (node->symbol->datatype == DATATYPE_CHAR)  ||
             (node->symbol->datatype == DATATYPE_FLOAT) ||
             (node->symbol->type == SYMBOL_INTEGER)     || 
             (node->symbol->type == SYMBOL_CHAR)        ||
             (node->symbol->type == SYMBOL_FLOAT))
        )
        ||
        ((node->type == AST_FUNCTION_CALL) && (node->symbol) && (getParamCount(node) == node->symbol->auxnum) &&
            ((node->symbol->datatype == DATATYPE_INT)   || 
             (node->symbol->datatype == DATATYPE_CHAR)  ||
             (node->symbol->datatype == DATATYPE_FLOAT))
        )
    ) return 1;

    return 0;
}

int expressionIsInteger(AST *node)
{
    if (node == 0){
        return 0;
    }

    if ( (node->type == AST_ADD) ||
         (node->type == AST_SUB) ||
         (node->type == AST_MUL) ||
         (node->type == AST_DIV))
    {
        return expressionIsInteger(node->son[0]) * expressionIsInteger(node->son[1]);
    }
    else if  (((node->type == AST_SYMBOL)        && ((node->symbol->type     == SYMBOL_INTEGER) ||
                                                     (node->symbol->type     == SYMBOL_CHAR)))
              ||
              (( (node->type == AST_SYMBOL) || (node->type == AST_ARRAY_ACCESS)) && (node->symbol->datatype != DATATYPE_FLOAT)) ||
              ((node->type == AST_FUNCTION_CALL) && (node->symbol->datatype != DATATYPE_FLOAT))) 
    {   
            // Verificar se os arrays de dentro também tem indices válidos
        if (node->type == AST_ARRAY_ACCESS)
            return expressionIsInteger(node->son[1]);

        return 1;
    }

    else return 0;
}

int getParamCount(AST *node)
{
    int paramCount = 0;

    if (node->type == AST_FUNCTION_CALL)
    {
        if (node->son[1])
        {
            paramCount = 1;

            AST* auxnode;
            auxnode = node->son[1];

            while (auxnode)
            {
                paramCount++;
                auxnode = auxnode->son[1];
            }
        }
    }
    return paramCount;
}

void check_undeclared()
{
    SemanticErrors += hash_check_undeclared();
}

////////////////////////////////////////////////////////////
// Checar operandos
////////////////////////////////////////////////////////////

void check_operands(AST *node)
{
    if (node == 0){
        return;
    }

    switch (node->type)
    {
        case AST_ADD: 
            if( (!check_valid_arithmetical_operand(node->son[0])) || 
                (!check_valid_arithmetical_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for ADD\n");
                SemanticErrors++;
            }
            break; 
        case AST_SUB: 
            if( (!check_valid_arithmetical_operand(node->son[0])) || 
                (!check_valid_arithmetical_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for SUB\n");
                SemanticErrors++;
            }
            break; 
        case AST_MUL: 
            if( (!check_valid_arithmetical_operand(node->son[0])) || 
                (!check_valid_arithmetical_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for MUL\n");
                SemanticErrors++;
            }
            break; 
        case AST_DIV: 
            if( (!check_valid_arithmetical_operand(node->son[0])) || 
                (!check_valid_arithmetical_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for DIV\n");
                SemanticErrors++;
            }
            break; 
        case AST_L: 
            if( (!check_valid_boolean_operand(node->son[0])) || 
                (!check_valid_boolean_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for L\n");
                SemanticErrors++;
            }
            break; 
        case AST_G: 
            if( (!check_valid_boolean_operand(node->son[0])) || 
                (!check_valid_boolean_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for G\n");
                SemanticErrors++;
            }
            break; 
        case AST_LE: 
            if( (!check_valid_boolean_operand(node->son[0])) || 
                (!check_valid_boolean_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for LE\n");
                SemanticErrors++;
            }
            break; 
        case AST_GE: 
            if( (!check_valid_boolean_operand(node->son[0])) || 
                (!check_valid_boolean_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for GE\n");
                SemanticErrors++;
            }
            break; 
        case AST_EQ: 
            if( (!check_valid_boolean_operand(node->son[0])) || 
                (!check_valid_boolean_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for EQ\n");
                SemanticErrors++;
            }
            break; 
        case AST_DIF: 
            if( (!check_valid_boolean_operand(node->son[0])) || 
                (!check_valid_boolean_operand(node->son[1])) )
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for DIF\n");
                SemanticErrors++;
            }
            break; 
        case AST_CAPSULE:
            if(!check_valid_arithmetical_operand(node->son[0]) && !check_valid_boolean_operand(node->son[0]))
            {
                fprintf(stderr, "Semantic ERROR: invalid operands for Expression inside \"()\"\n");
                SemanticErrors++;
            }
            break; 
    }

}

void checkNativeFunctions(AST *node)
{
    if (node == 0){
        return;
    }

    if(node->type == AST_WHILE)
    {
        if (!(node->son[0] && check_valid_boolean_operand(node->son[0])))
        {
            fprintf(stderr, "Semantic ERROR: invalid operand for while call\n");
            SemanticErrors++;
        }
    }

    if(node->type == AST_IF)
    {
        if (!(node->son[0] && check_valid_boolean_operand(node->son[0])))
        {
            fprintf(stderr, "Semantic ERROR: invalid operand for IF call\n");
            SemanticErrors++;
        }
    }

    if(node->type == AST_RETURN)
    {
        if (!(node->son[0] && check_valid_arithmetical_operand(node->son[0])))
        {
            fprintf(stderr, "Semantic ERROR: cant return a bool value\n");
            SemanticErrors++;
        }
    }

    for (int i = 0; i< MAX_SONS; i++){
        checkNativeFunctions(node->son[i]);
    }
}

void check_and_optimize_loops(AST *all, AST *node)
{
    if (node == 0){
        return;
    }

    if(node->type == AST_WHILE)
    {
        // Referenciar variável de índice no while
        node->symbol->aux = check_optimizible_loop(node->son[0]);
        
        if (node->symbol->aux)
        {        
            if (check_loop_index_dec(node) && check_assignment_count(node, all) == 1)
            {
                // Calcula o número de repetições
                node->symbol->auxnum = node->symbol->aux->init - node->symbol->aux->auxnum;
                
            }
            // Resetar o número auxilhar que utilizamos
            node->symbol->aux->auxnum = 0;
        }
    }

    for (int i = 0; i< MAX_SONS; i++){
        check_and_optimize_loops(all, node->son[i]);
    }
}

HASH_NODE *check_optimizible_loop(AST *node)
{
    if (!node) return 0;

    while (node->type == AST_CAPSULE)
        node = node->son[0];

    if (node->son[0] && node->son[1])
    {
        if (node->type == AST_G)
        {
            if (
                ((node->son[0]->type == AST_SYMBOL) && (node->son[0]->symbol->type == SYMBOL_VARIABLE)) &&
                ((node->son[1]->type == AST_SYMBOL) && (node->son[1]->symbol->type == SYMBOL_INTEGER)))
            {
                node->son[0]->symbol->auxnum = atoi(node->son[1]->symbol->text);
                return node->son[0]->symbol;
            }
        }
    }
    return 0;
}

int check_loop_index_dec(AST *node)
{
    AST *aux = node->son[1]; 
    AST *var = 0;
    AST *num = 0;
    AST *exp = 0;
    int found = 0;

    if (!aux) return 0;

    if (aux->type == AST_BODY)
        aux = aux->son[0];

    if (!aux) return 0;   

    while (aux && !found){ 
        if (aux->type == AST_COMMAND_LIST){
            if (aux->son[0] && aux->son[0]->type == AST_ASSIGN) 
            {
                // LADO DIREITO: assegurar que é o mesmo índice do while
                var = aux->son[0]->son[0];
                found = (var && var->symbol && var->symbol == node->symbol->aux);

                // LADO ESQUERDO: assegurar que expressão é "index - 1"
                exp = aux->son[0]->son[1];
                var = exp->son[0];
                num = exp->son[1];
                
                found = found && (exp && var && num);
                found = found && (var->symbol && var->symbol == node->symbol->aux);
                found = found && (exp->type == AST_SUB);
                found = found && (num->symbol && num->symbol->type == SYMBOL_INTEGER && atoi(num->symbol->text) == 1);

                if (found) return 1;
            }
        }

        if (aux->son[1] && aux->son[1]->type == AST_COMMAND_LIST) aux = aux->son[1];
        else aux = 0;
    }

    return 0;
}

int check_assignment_count(AST *while_node, AST *node)
{
    int result = 0;
    AST *var = 0;

    if (node == 0){
        return 0;
    }

    if(node->type == AST_ASSIGN)
    {
        var = node->son[0];
        if (var && var->symbol && var->symbol == while_node->symbol->aux) 
            return 1;
    }

    for (int i = 0; i< MAX_SONS; i++){
        result += check_assignment_count(while_node, node->son[i]);
    }

    return result;
}