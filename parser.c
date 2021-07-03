#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "symbol.h"
#include "lexer.h"

#include "utils.h"

#define NB_TYPES 2

symbol_t* global_table = NULL;

val_types_t stringToType(char* type)
{
    if(strcmp(type, "entier") == 0)
        return ENTIER;
    
    return RIEN;
}

bool isValidType(char* type)
{
    if(strcmp(type, "entier") == 0
    || strcmp(type, "rien") == 0)
        return true;
        
    return false;
}

void parser(buffer_t* buffer)
{
    while ( !buf_eof(buffer) )
    {
        if(strcmp(lexer_getalphanum(buffer), "fonction") != 0)
            syntax_error("Function expected");
        else
        {
            ast_t * func = analyse_fonction(buffer);
            if(sym_search(global_table, func->function.name))
                syntax_error("Duplicate function name !");
            
            sym_add(&global_table, sym_new(func->function.name, SYM_FUNCTION, func));
        }
    }   
}

ast_t* analyse_fonction(buffer_t* buffer)
{
    symbol_t* func_table = NULL;
    char* f_name = lexer_getalphanum(buffer);
    ast_list_t* args = analyse_parametres(buffer, func_table);
    val_types_t reType = analyse_type_de_retour(buffer);
    ast_list_t* stmnt = analyse_corps_de_fonction(buffer, func_table);
    
        buf_print(buffer);

    return ast_new_function(f_name, reType, args, stmnt, func_table);
}

ast_list_t* analyse_parametres(buffer_t* buffer, symbol_t* func_table)
{
    ast_list_t* args = NULL;
    buf_skipblank(buffer);
    lexer_assert_openbrace (buffer, "Expected '(' after function name !\n");

    while (true)
    {
        char* type = lexer_getalphanum(buffer);
        char nextChar = buf_getchar_rollback(buffer);
        if(strlen(type) == 0 &&  nextChar == ')')
        {
            buf_forward(buffer, 1);
            return args;
        }

        if ( !isValidType(type))
            syntax_error("Uknown type !");
        
        char* nom = lexer_getalphanum(buffer);

        if (sym_search(func_table, nom))
            syntax_error("Duplicate parametre name !");

        sym_add(&func_table, sym_new(nom, stringToType(type), ast_new_variable(nom, stringToType(type)))),
        ast_list_add(&args, ast_new_variable(nom, stringToType(type)));
                
        nextChar = buf_getchar_after_blank(buffer);
        if(nextChar == ')')
            return args;
        if (nextChar != ',')
            syntax_error("Wrong parametres separator !"); 
    }
}

val_types_t analyse_type_de_retour(buffer_t* buffer)
{
    buf_skipblank(buffer);
    lexer_assert_twopoints (buffer, "Expected ':' after function parameters !\n");
    char* type = lexer_getalphanum(buffer);
        if ( !isValidType(type))
            syntax_error("Uknown type !");
    
    return stringToType(type);
}

ast_list_t* analyse_corps_de_fonction(buffer_t* buffer, symbol_t* func_table)
{
    buf_skipblank(buffer);
    lexer_assert_openbracket(buffer, "Expected '{' after function signature !\n");

    ast_list_t* res = NULL;
    while (true)
    {
        char* next = lexer_getalphanum(buffer);
        if (strcmp(next, "entier") == 0)
        {
            char* nextInstru = lexer_getuntil(buffer, ';');
            printf(COLOR_RED "Get Until : %s\n" COLOR_DEFAULT, nextInstru);
        }
        else if (strcmp(next, "si") == 0)
        {
            ast_list_add(&res, analyse_condition(buffer, func_table));
        }
        else if (strcmp(next, "tanque") == 0)
        {
            /* code */
        }
        else if (strcmp(next, "function") == 0)
            syntax_error("Not function allowed in function !");
        
        if (buf_getchar(buffer) == '}')
            return res;        
    }
    
}

ast_t* analyse_condition(buffer_t *buffer, symbol_t* func_table)
{
    
    buf_skipblank(buffer);
    lexer_assert_openbrace (buffer, "Expected '(' after 'si' key !\n");

    return ast_new_condition(analyse_condition_critere(buffer, func_table), NULL, NULL);
    
}

ast_t* analyse_condition_critere(buffer_t* buffer, symbol_t* func_table)
{
    ast_t* binary = NULL;

    /* code */

    char nextChar = buf_getchar_after_blank(buffer);
    if (nextChar == '(')
        binary->binary.left = analyse_condition_critere(buffer, func_table); 
        
    if(nextChar == ')')
        return binary;
    
}

ast_t* analyse_expression(buffer_t* buffer, symbol_t* func_table)
{
    bool braceOpen = false;
    char c = buf_getchar_rollback(buffer);
    ast_list_t* stack = NULL;
    ast_t* res = NULL;

    if(c == '(')
        ast_list_add(&stack, analyse_expression(buffer, func_table));
    if(c == ')')
        return res;
    else if (c == '-' || isdigit(c))
    {
        ast_list_add(&stack, ast_new_integer(lexer_getnumber(buffer)));
    }
    else
    {
        char* next = lexer_getalphanum(buffer);

    }
}

ast_list_t* translate_to_list(ast_list_t* chaine)
{
    ast_list_t* pile = NULL;
    ast_list_t* sortie = NULL;
    
    ast_list_add(&pile, NULL);
    while (1)
    {
        if (chaine->value == NULL && pile->value == NULL)
            return sortie;
        else
        {
            ast_t* a = pile->value; 
            ast_t* b = chaine->value; 
            
            if (priority(a, b) == -1)
            {
                ast_list_add(&pile, b);
                chaine = chaine->next;
            }
            else
            {
                do
                {
                    ast_list_add(&sortie, ast_list_pop(&pile)->value);
                } while (priority(pile->value, sortie->value) != -1);
            }
        }
    }
    free_ast_list(pile);
}

int priority(ast_t* c1, ast_t* c2)
{
    // on gère le cas du \0
    if(c1 == NULL)
        return -1;
    //on gère le cas d'un entier 
    if(c1->type == AST_INTEGER || c1->type == AST_VARIABLE)
        return 1;

    //On gère le cas du + et -
    if(c1->type == AST_BINARY && (c1->binary.op == OP_PLUS || c1->binary.op == OP_MOIN)){
        if(c2->type = AST_INTEGER || c2->type == AST_BINARY && c2->binary.op == OP_FOIS) 
            return -1;
        
        return 1;
    }
    //on gère le cas du *
    if(c1->type == AST_BINARY && c1->binary.op == OP_FOIS){
        if(c2->type = AST_INTEGER ) 
            return -1;
        
        return 1;
    }
}

