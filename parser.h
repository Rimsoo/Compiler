#ifndef PARSER_H
#define PARSER_H
#include <stdbool.h>
#include "buffer.h"
#include "ast.h"
   
void parser(buffer_t *buffer);
ast_t* analyse_fonction(buffer_t *buffer);
ast_list_t* analyse_parametres(buffer_t* buffer, symbol_t* func_table);
val_types_t analyse_type_de_retour(buffer_t* buffer);
ast_list_t* analyse_corps_de_fonction(buffer_t* buffer, symbol_t* func_table);
ast_t* analyse_condition(buffer_t *buffer, symbol_t* func_table);
ast_t* analyse_condition_critere(buffer_t* buffer, symbol_t* func_table);

int priority(ast_t* c1, ast_t* c2);

bool isValidType(char* type);
val_types_t stringToType(char* type);


#endif