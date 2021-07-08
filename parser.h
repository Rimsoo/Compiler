#ifndef PARSER_H
#define PARSER_H
#include <stdbool.h>
#include "buffer.h"
#include "ast.h"
   
void parser(buffer_t *buffer);
ast_t* analyse_fonction(buffer_t *buffer);
ast_list_t* analyse_parametres(buffer_t* buffer, symbol_t** func_table);
val_types_t analyse_type_de_retour(buffer_t* buffer);
ast_list_t* analyse_corps_de_fonction(buffer_t* buffer, symbol_t** func_table);
ast_t* analyse_condition(buffer_t *buffer, symbol_t* func_table);
ast_t* analyse_expression(buffer_t* buffer, symbol_t* func_table);
ast_t* analyse_appel_fonction(buffer_t* buffer, symbol_t* func_table, symbol_t* called_func);
ast_list_t* analyse_corps_de_condition(buffer_t* buffer, symbol_t** func_table);
ast_t* polonaise_to_tree(ast_list_t* p);
int priority(ast_t* c1, ast_t* c2);

bool isValidType(char* type);
val_types_t stringToType(char* type);
char *typeToString(val_types_t type);
char *binaryEnumToString(ast_binary_e op);

// write to language
char *mapper(char *key);
void writer(ast_list_t* ast);
char* write_function(ast_t* fn);
char* write_block_content(ast_list_t *content);
char* write_declaration(ast_t* lvalue, ast_t* rvalue); 
char* write_expression(ast_t* exp);
char *write_assignment(ast_t *lvalue, ast_t *rvalue);
char *write_condition(ast_t *branch);

#endif