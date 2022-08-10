#include "ast.h"

#ifndef SEMANTIC_HEADER
#define SEMANTIC_HEADER

extern int SemanticErrors;
void setNodeType(AST *node, int symbol);

void check_assignments(AST *node);
void check_and_set_declarations(AST *node);
void check_undeclared();
void check_operands(AST *node);
void check_and_optimize_loops(AST *all, AST *node);

int check_valid_arithmetical_operand(AST *node);
int check_valid_boolean_operand(AST *node);
HASH_NODE* check_optimizible_loop(AST *node);
int check_loop_index_dec(AST *node);
int check_assignment_count(AST *while_node, AST *node);
int expressionIsInteger(AST *node);
int getParamCount(AST *node);
void checkNativeFunctions(AST *node);

int check_function_operands(AST *node);

#endif