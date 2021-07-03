#include <stdlib.h>
#include <string.h>
#include "symbol.h"

symbol_t *sym_new(char *name, symbol_type type, ast_t *attributes)
{
    symbol_t * res = (symbol_t *)malloc(sizeof(symbol_t ));
    res->type = type;
    res->name = name;
    res->attributes = attributes;
    res->next = NULL;

    return res;
}

void sym_delete(symbol_t *sym)
{
    free(sym->name);
    // while (sym->attributes)
    // {
    //     ast_t *attributes
    // }

    free(sym);
    
}

void sym_add(symbol_t **table, symbol_t *sym)
{
    sym->next = *table;
    *table = sym;
}

void sym_remove(symbol_t **table, symbol_t *sym)
{
    if (*table == NULL)
        return;
    
    symbol_t* tmp = *table;
    symbol_t* tmp_prev = NULL;
    symbol_t* tmp_next;

    while (tmp)
    {
        if (tmp == sym)
        {
            if (!tmp_prev)
            {
                sym_delete(sym);
            }
            else
            {
                tmp_next = tmp->next;
                tmp_prev->next = tmp_next;
                
                sym_delete(sym);
                tmp = tmp_prev;
            }  
            return;          
        }
        tmp_prev = tmp;
        tmp = tmp->next;
    }   
}

symbol_t *sym_search(symbol_t *table, char *name)
{
    symbol_t* tmp = table;
    while (tmp)
    {
        if (strcmp(tmp->name , name) == 0)
            return tmp;

        tmp = tmp->next;
    }
    
    return NULL;
}