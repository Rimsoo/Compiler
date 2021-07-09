#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"

ast_list_t *ast_list_new_node(ast_t *elem)
{
    ast_list_t * res = (ast_list_t *)malloc(sizeof(ast_list_t));
    res->value = elem;
    res->next = NULL;

    return res;
}

ast_list_t *ast_list_add(ast_list_t **list, ast_t *elem)
{
    ast_list_t *new = ast_list_new_node(elem);

    new->next = *list;
    *list = new;

    return *list;
}

ast_list_t *ast_list_pop(ast_list_t **list)
{
    if( !*list )
        return NULL;

    ast_list_t *res = *list;

    *list = (*list)->next;
    res->next = NULL; 

    return res;
}

ast_t *ast_new_variable(char *name, val_types_t type)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_VARIABLE;

    res->var.type = type;
    res->var.name = name;

    return res;
}

ast_t *ast_new_function(char *name, val_types_t return_type, ast_list_t *params, ast_list_t *stmts, symbol_t* func_table)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_FUNCTION;

    res->function.name = name;
    res->function.params = params;
    res->function.return_type = return_type;
    res->function.symbols = func_table;
    res->function.stmts = stmts;

    return res;
}

ast_t *ast_new_condition(ast_t *condition, ast_t *valid, ast_t *invalid)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_CONDITION;

    res->branch.condition = condition;
    res->branch.valid = valid;
    res->branch.invalid = invalid;

    return res;
}

ast_t *ast_new_binary(ast_binary_e op, ast_t *left, ast_t *right)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_BINARY;

    res->binary.op = op;
    res->binary.left = left;
    res->binary.right = right;

    return res;
}

ast_t *ast_new_integer(long val)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_INTEGER;

    res->integer = val;
    
    return res;
}

ast_t *ast_new_fncall(char *name, ast_list_t *args)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_FNCALL;

    res->call.name = name;
    res->call.args = args;
    
    return res;
}

ast_t *ast_new_assignment(ast_t *lvalue, ast_t *rvalue)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_ASSIGNMENT;

    res->assignment.lvalue = lvalue;
    res->assignment.rvalue = rvalue;
    
    return res;
}

ast_t *ast_new_declaration(ast_t *lvalue, ast_t *rvalue)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_DECLARATION;

    res->assignment.lvalue = lvalue;
    res->assignment.rvalue = rvalue;
    
    return res;
}

ast_t *ast_new_return(ast_t *expr)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_RETURN;

    res->ret.expr = expr;
    
    return res;
}

ast_t *ast_new_comp_stmt(ast_list_t *stmts)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_COMPOUND_STATEMENT;

    res->compound_stmt.stmts = stmts;
    return res;
}

ast_t *ast_new_unary(ast_binary_e op, ast_t *operand)
{
    ast_t* res = (ast_t*)malloc(sizeof(ast_t));
    res->type = AST_UNARY;

    res->unary.op = op;
    res->unary.operand = operand;

    return res;
}

void free_ast_list(ast_list_t* liste)
{
    if(liste != NULL){
        free_ast_list(liste->next);
        free(liste);
    }
}

void list_print(ast_list_t* liste)
{
    if (!liste || !liste->value)
    {
        printf(" X\n");
        return;
    }   
    
    switch (liste->value->type)
    {
    case AST_VARIABLE:
        printf(" %s ->", liste->value->var.name);
        break;
    
    case AST_INTEGER:
        printf(" %ld ->", liste->value->integer);
        break;

    case AST_FNCALL:
        printf(" %s ->", liste->value->call.name);
        break;

    case AST_BINARY:
        printf(" [%d] ->", liste->value->binary.op);
        break;
    default:
        break;
    }
    list_print(liste->next);
}