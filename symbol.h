#ifndef SYM_H
#define SYM_H
#include "ast.h"

typedef enum symbol_type
{
    SYM_VARIABLE,
    SYM_FUNCTION
}symbol_type;

typedef struct symbol_t
{
    char* name;
    enum symbol_type type;
    struct ast_t *attributes;
    struct symbol_t* next;
}symbol_t;

symbol_t *sym_new(char *name, symbol_type type, struct ast_t *attributes);
void sym_delete(symbol_t *sym);
void sym_remove(symbol_t **table, symbol_t *sym);
void sym_add(symbol_t **table, symbol_t *sym);
symbol_t *sym_search(symbol_t *table, char *name);

#endif