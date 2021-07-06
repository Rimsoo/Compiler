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

ast_binary_e binaryStringToEnum(char* op)
{
    if(strcmp(op, "+") == 0)
        return OP_PLUS;
    if(strcmp(op, "-") == 0)
        return OP_MOINS;
    if(strcmp(op, "*") == 0)
        return OP_FOIS;
    if(strcmp(op, "/") == 0)
        return OP_DIVISE;
    if(strcmp(op, "==") == 0)
        return OP_DOUBLE_EGALE;
    if(strcmp(op, "!=") == 0)
        return OP_PAS_EGALE;
    if(strcmp(op, ">") == 0)
        return OP_SUPPERIEUR;
    if(strcmp(op, "<") == 0)
        return OP_INFERIEUR;
    if(strcmp(op, ">=") == 0)
        return OP_SUPPERIEUR_EGALE;
    if(strcmp(op, "<=") == 0)
        return OP_INFERIEUR_EGALE;
    if(strcmp(op, "ET") == 0)
        return OP_ET;
    if(strcmp(op, "OU") == 0)
        return OP_OU;
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
    ast_list_t* args = analyse_parametres(buffer, &func_table);
    val_types_t reType = analyse_type_de_retour(buffer);
    ast_list_t* stmnt = analyse_corps_de_fonction(buffer, &func_table);
    
        

    return ast_new_function(f_name, reType, args, stmnt, func_table);
}

ast_list_t* analyse_parametres(buffer_t* buffer, symbol_t** func_table)
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

        if (sym_search(*func_table, nom))
            syntax_error("Duplicate parametre name !");

        sym_add(func_table, sym_new(nom, stringToType(type), ast_new_variable(nom, stringToType(type)))),
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

ast_list_t* analyse_corps_de_fonction(buffer_t* buffer, symbol_t** func_table)
{
    buf_skipblank(buffer);
    lexer_assert_openbracket(buffer, "Expected '{' after function signature !\n");

    ast_list_t* res = NULL;
    while (true)
    {
        char* next = lexer_getalphanum(buffer);
        buf_print(buffer);
        if (isValidType(next))
        {
            char* varName = lexer_getalphanum(buffer);
            if(sym_search(*func_table, varName))
                syntax_error("Variable already exist");

            char endOrAssing = buf_getchar_after_blank(buffer);
            ast_t* rvalue = NULL;
            if (endOrAssing == '=')
                rvalue = analyse_expression(buffer, *func_table);
            
            ast_t* newVar = ast_new_declaration(ast_new_variable(varName, stringToType(next)), rvalue);
            ast_list_add(&res, newVar);
            sym_add(func_table, sym_new(varName, SYM_VARIABLE, newVar->declaration.lvalue));
        }
        else if (strcmp(next, "si") == 0)
        {
            ast_list_add(&res, analyse_condition(buffer, *func_table));
        }
        else if (strcmp(next, "tanque") == 0)
        {
            /* TODO */
        }
        else if (strcmp(next, "retourner") == 0)
            ast_list_add(&res, ast_new_return(analyse_expression(buffer, *func_table)));

        else if (strcmp(next, "function") == 0)
            syntax_error("Not function allowed in function !");
        
        else if(sym_search(*func_table, next)) // assignation
        {
            char endOrAssing = buf_getchar_after_blank(buffer);
            ast_t* rvalue = NULL;
            if (endOrAssing != '=')
                syntax_error("Symbol '=' needed for assignment !");
            
            rvalue = analyse_expression(buffer, *func_table);
            
            ast_t* newAssing = ast_new_assignment(sym_search(*func_table, next)->attributes, rvalue);
            ast_list_add(&res, newAssing);
        }
        else
            syntax_error("Unknow symbol name !");
        
        if (buf_getchar_rollback(buffer) == '}')
        {
            buf_forward(buffer, 1);
            return res;
        }            
    }
    
}

ast_t* analyse_condition(buffer_t *buffer, symbol_t* func_table)
{
    
    buf_skipblank(buffer);
    lexer_assert_openbrace (buffer, "Expected '(' after 'si' key !\n");

    return ast_new_condition(analyse_expression(buffer, func_table), NULL, NULL);
}

ast_t* analyse_expression(buffer_t* buffer, symbol_t* func_table)
{
    bool expEnd = true;
    ast_list_t* stack = NULL;
    ast_list_add(&stack, NULL);

    while(expEnd)
    {
        char c = buf_getchar_rollback(buffer);

        if(c == '(')
        {
            buf_forward(buffer, 1);
            ast_list_add(&stack, analyse_expression(buffer, func_table));
        }
        else if(c == ')' || c == ';')
        {
            buf_forward(buffer, 1);
            expEnd = false;
        }    
        else if (c == '-' || isdigit(c))
        {
            ast_list_add(&stack, ast_new_integer(lexer_getnumber(buffer)));
        }
        else if (isop(c))
        {
            ast_list_add(&stack, ast_new_binary(binaryStringToEnum(lexer_getop(buffer)), NULL, NULL));
        }
        else
        {
            char* next = lexer_getalphanum(buffer);
            symbol_t* found;

            if( (found = sym_search(func_table, next)) )
                ast_list_add(&stack, found->attributes);

            else if ( (found = sym_search(global_table, next)))         
                ast_list_add(&stack, analyse_appel_fonction(buffer, func_table, found));
                        
            else
                syntax_error("Unknow variable");
            
        }
    }  
    
    return  list_to_tree(translate_to_list(stack));
}

ast_t* analyse_appel_fonction(buffer_t *buffer, symbol_t* func_table, symbol_t* called_func)
{
    lexer_assert_openbrace(buffer, "Expected open brace after function call !");
    ast_list_t* arg_list = NULL;
    ast_list_t* args_count = called_func->attributes->function.params;

    while (args_count)
    {
        char* next = lexer_getalphanum(buffer);
        symbol_t* found;

        if(isnumber(next))
            ast_list_add(&arg_list, ast_new_integer(strtol(next, NULL, 10)));

        else if( (found = sym_search(func_table, next)) )
            ast_list_add(&arg_list, found->attributes);

        else if ( (found = sym_search(global_table, next)))         
            ast_list_add(&arg_list, analyse_appel_fonction(buffer, func_table, found));
                    
        else
            syntax_error("Unknow variable");

        args_count = args_count->next;
        if(args_count)
            lexer_assert_simplechar(buffer, ',', "Wrong parameter separator !");
    }

    lexer_assert_closebrace(buffer, "Expected close brace after function call !");

    return ast_new_fncall(called_func->name, arg_list);
}

ast_t* list_to_tree(ast_list_t* p)
{
    if (!p)
        return NULL;
    ast_list_t* n = ast_list_pop(&p);
    if ( p && p->value && p->value->type == AST_BINARY)
    {        
        p->value->binary.right = list_to_tree(p);
        p->value->binary.left = list_to_tree(p);
    }
    
    return n->value;
}

ast_list_t* translate_to_list(ast_list_t* chaine)
{
    ast_list_t* pile = NULL;
    ast_list_t* sortie = NULL;
    ast_list_add(&sortie, NULL);
    ast_list_add(&pile, NULL);
    while (true)
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
    if(c1->type == AST_BINARY && (c1->binary.op == OP_PLUS || c1->binary.op == OP_MOINS)){
        if(c2 != NULL && (c2->type == AST_INTEGER || c2->type == AST_BINARY && c2->binary.op == OP_FOIS)) 
            return -1;
        
        return 1;
    }
    //on gère le cas du *
    if(c1->type == AST_BINARY && c1->binary.op == OP_FOIS){
        if(c2 != NULL && c2->type == AST_INTEGER ) 
            return -1;
        
        return 1;
    }
}

