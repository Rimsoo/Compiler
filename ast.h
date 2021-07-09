#ifndef AST_H
#define AST_H
#include <stdbool.h>
#include "symbol.h"

typedef enum ast_node_type_e
{
    AST_VOID,
    AST_INTEGER,
    AST_BINARY,
    AST_UNARY,
    AST_FUNCTION,
    AST_FNCALL,
    AST_VARIABLE,
    AST_CONDITION,
    AST_LOOP,
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_COMPOUND_STATEMENT,
    AST_RETURN
} ast_node_type_e;

typedef enum ast_binary_e
{
    OP_PLUS,
    OP_MOINS,
    OP_FOIS,
    OP_DIVISE,
    OP_DOUBLE_EGALE,
    OP_PAS_EGALE,
    OP_SUPPERIEUR,
    OP_INFERIEUR,
    OP_SUPPERIEUR_EGALE,
    OP_INFERIEUR_EGALE,
    OP_ET,
    OP_OU,
    OP_PARENTHESES
}ast_binary_e;

typedef struct ast_list_t
{
    struct ast_t* value;
    struct ast_list_t* next;
}ast_list_t;

typedef enum val_types_t
{
    ENTIER = 0, 
    RIEN   
}val_types_t;

typedef struct ast_t
{
    ast_node_type_e type;
    union
    {
        long integer;
        struct
        {
            char *name;
            val_types_t type;
        } var;
        struct
        {
            ast_binary_e op;
            struct ast_t *left;
            struct ast_t *right;
        } binary;
        struct
        {
            char op;
            struct ast_t *operand;
        } unary;
        struct
        {
            char *name;
            struct ast_list_t *args;
        } call;
        struct
        {
            char *name;
            val_types_t return_type;
            struct ast_list_t *params;
            struct ast_list_t *stmts;
            struct symbol_t* symbols;
        } function;
        struct
        {
            struct ast_list_t *stmts;
        } compound_stmt;
        struct
        {
            struct ast_t *lvalue;
            struct ast_t *rvalue;
        } assignment;
        struct
        {
            struct ast_t *lvalue;
            struct ast_t *rvalue;
        } declaration;
        struct
        {
            struct ast_t *condition;
            struct ast_t *valid;
            struct ast_t *invalid;
        } branch;
        struct
        {
            struct ast_t *condition;
            struct ast_t *stmt;
        } loop;
        struct
        {
            struct ast_t *expr;
        } ret;
    };
} ast_t;

ast_t *ast_new_integer(long val);
ast_t *ast_new_variable(char *name, val_types_t type);
ast_t *ast_new_binary(ast_binary_e op, ast_t *left, ast_t *right);
ast_t *ast_new_unary(ast_binary_e op, ast_t *operand);
ast_t *ast_new_function(char *name, val_types_t return_type, ast_list_t *params, ast_list_t *stmts, struct symbol_t* func_table);
ast_t *ast_new_fncall(char *name, ast_list_t *args);
ast_t *ast_new_comp_stmt(ast_list_t *stmts);
ast_t *ast_new_assignment(ast_t *lvalue, ast_t *rvalue);
ast_t *ast_new_declaration(ast_t *lvalue, ast_t *rvalue);
ast_t *ast_new_condition(ast_t *condition, ast_t *valid, ast_t *invalid);
ast_t *ast_new_loop(ast_t *condition, ast_t *stmt);
ast_t *ast_new_return(ast_t *expr);
ast_list_t *ast_list_new_node(ast_t *elem);
ast_list_t *ast_list_add(ast_list_t **list, ast_t *elem);
ast_list_t *ast_list_pop(ast_list_t **list);
void free_ast_list(ast_list_t* liste);
void list_print(ast_list_t* liste);

#endif