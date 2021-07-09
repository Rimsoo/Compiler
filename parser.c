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

char* typeToString(val_types_t type)
{
    switch (type)
    {
    case ENTIER:
        return "entier";
    
    case RIEN: default:
        return "rien";
    }
}

bool isValidType(char* type)
{
    if(strcmp(type, "entier") == 0
    || strcmp(type, "rien") == 0)
        return true;
        
    return false;
}

char *binaryEnumToString(ast_binary_e op)
{
    switch (op)
    {
        case OP_PLUS:
			return " + ";
        case OP_MOINS:
            return " - ";
        case OP_FOIS:
            return " * ";
        case OP_DIVISE:
            return " / ";
        case OP_DOUBLE_EGALE:
            return " == ";
        case OP_PAS_EGALE:
            return " != ";
        case OP_SUPPERIEUR:
            return " > ";
        case OP_INFERIEUR:
            return " < ";
        case OP_SUPPERIEUR_EGALE:
            return " >= ";
        case OP_INFERIEUR_EGALE:
            return " <= ";
        case OP_ET:
			return " && ";
        case OP_OU: default:
            return " || ";
    }
}

ast_binary_e binaryStringToEnum(char *op)
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
    ast_list_t* ast = NULL;
    while ( !buf_eof(buffer) )
    {
        if(strcmp(lexer_getalphanum(buffer), "fonction") != 0)
            syntax_error("Function expected");
        else
        {
            ast_t * func = analyse_fonction(buffer);
            sym_add(&global_table, sym_new(func->function.name, SYM_FUNCTION, func));
            ast_list_add(&ast, func);
        }
    }   

    writer(ast);
    printf(COLOR_GREEN"Code compiled succesfully !\n");
}

ast_t* analyse_fonction(buffer_t* buffer)
{
    symbol_t* func_table = NULL;
    char* f_name = lexer_getalphanum(buffer);
    if(sym_search(global_table, f_name))
        syntax_error("Duplicate function name !");
            
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

        sym_add(func_table, sym_new(nom, SYM_VARIABLE, ast_new_variable(nom, stringToType(type)))),
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
        else if (strcmp(next, "tantque") == 0)
        {
            /* TODO */
            lexer_getuntil(buffer, ';'); 
            buf_forward(buffer, 1);
            buf_getchar_after_blank(buffer);
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
        else if (strcmp(next, "sinon") == 0); // managed in analyse_condition
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
    ast_t* condition = analyse_expression(buffer, func_table);
    ast_t* valid = ast_new_comp_stmt(analyse_corps_de_condition(buffer, &func_table));
    ast_t* invalid = NULL;

    char* next = lexer_getalphanum_rollback(buffer);
    if(strcmp(next, "sinon") == 0)
    {
        lexer_getalphanum(buffer);
        char* getSi = lexer_getalphanum_rollback(buffer);
        if(strcmp(getSi, "si") == 0)
        {
            lexer_getalphanum(buffer);
            invalid = analyse_condition(buffer, func_table);
        }
        else
            invalid = ast_new_comp_stmt(analyse_corps_de_condition(buffer, &func_table));
    }

    return ast_new_condition(condition, valid, invalid);
}

ast_t* analyse_expression(buffer_t* buffer, symbol_t* func_table)
{
    bool expEnd = true;
    ast_list_t* result_stack = NULL;
    ast_list_t* operator_stack = NULL;
    ast_list_add(&operator_stack, NULL);

    while(expEnd)
    {
        char c = buf_getchar_rollback(buffer);

        if(c == '(')
        {
            buf_forward(buffer, 1);
            ast_list_add(&result_stack, ast_new_unary(OP_PARENTHESES, analyse_expression(buffer, func_table)));
        }
        else if(c == ')' || c == ';')
        {
            buf_forward(buffer, 1);
            while (operator_stack->value)
                ast_list_add(&result_stack, ast_list_pop(&operator_stack)->value);
            
            expEnd = false;
        }    
        else if (c == '-' || isdigit(c))
            ast_list_add(&result_stack, ast_new_integer(lexer_getnumber(buffer)));
        
        else if (isop(c))
        {
            ast_t* nextOp = ast_new_binary(binaryStringToEnum(lexer_getop(buffer)), NULL, NULL);
            if (priority(operator_stack->value, nextOp) == -1)
            {
                ast_list_add(&operator_stack, nextOp);
            }
            else
            {
                do
                {
                    ast_list_add(&result_stack, ast_list_pop(&operator_stack)->value);
                } while (priority(operator_stack->value, nextOp) != -1);          
            }
        }
        
        else
        {
            char* next = lexer_getalphanum(buffer);
            symbol_t* found;

            if( (found = sym_search(func_table, next)) )
                ast_list_add(&result_stack, found->attributes);

            else if ( (found = sym_search(global_table, next)))         
                ast_list_add(&result_stack, analyse_appel_fonction(buffer, func_table, found));
                        
            else
                syntax_error("Unknow variable");
        }
    }  
    
    return polonaise_to_tree(&result_stack);
}

ast_t* analyse_appel_fonction(buffer_t *buffer, symbol_t* func_table, symbol_t* called_func)
{
    lexer_assert_openbrace(buffer, "Expected open brace !");
    ast_list_t* arg_list = NULL;
    ast_list_t* args_count = called_func->attributes->function.params;

    while (args_count)
    {
        char* next = lexer_getalphanum(buffer);
        symbol_t* found;

        if(isNumber(next))
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

    lexer_assert_closebrace(buffer, "Expected close brace !");

    return ast_new_fncall(called_func->name, arg_list);
}

ast_list_t* analyse_corps_de_condition(buffer_t* buffer, symbol_t** func_table)
{
    symbol_t* func_table_tmp = *func_table;

    ast_list_t* res = analyse_corps_de_fonction(buffer, func_table);

    while( *func_table != func_table_tmp)
    {
        symbol_t *tmp_next = (*func_table)->next;
        sym_remove( func_table, *func_table);
        *func_table = tmp_next;
    }

    return res;
}

ast_t* polonaise_to_tree(ast_list_t** p) // TO FIX
{
    if (!*p)
        return NULL;
    ast_t* tmp = NULL;
    ast_t* curr = ast_list_pop(p)->value;
    if (curr->type == AST_UNARY)
    {
        tmp = curr->unary.operand;
        free(curr); 
        curr = tmp;
    }
    if (curr->type == AST_BINARY)
    {     
        if(curr->binary.right == NULL)   
            curr->binary.right = polonaise_to_tree(p);
        if(curr->binary.left == NULL)
           curr->binary.left = polonaise_to_tree(p);
    }
    
    return curr;
}

int priority(ast_t* c1, ast_t* c2)
{
    // on gère le cas du NULL
    if(c1 == NULL)
        return -1;
    if(c2 == NULL)
        return 1;
    //on gère le cas d'un entier, var ou func
    if(c1->type == AST_INTEGER || c1->type == AST_VARIABLE || c1->type == AST_FNCALL)
        return 1;

    //On gère le cas du + et -
    if(c1->type == AST_BINARY && (c1->binary.op == OP_PLUS || c1->binary.op == OP_MOINS)){
        if((c2->type == AST_INTEGER  || c2->type == AST_VARIABLE || c2->type == AST_FNCALL
        || c2->type == AST_BINARY && (c2->binary.op == OP_FOIS || c2->binary.op == OP_DIVISE))) 
            return -1;
        
        return 1;
    }
    //on gère le cas du * et /
    if(c1->type == AST_BINARY && (c1->binary.op == OP_FOIS || c1->binary.op == OP_DIVISE)){
        if(c2->type == AST_INTEGER || c2->type == AST_VARIABLE || c2->type == AST_FNCALL ) 
            return -1;
        
        return 1;
    }
    //on gère le cas du <, <=, > et => 
    if(c1->type == AST_BINARY && 
        ( c1->binary.op == OP_INFERIEUR  || c1->binary.op == OP_INFERIEUR_EGALE
        ||c1->binary.op == OP_SUPPERIEUR || c1->binary.op == OP_SUPPERIEUR_EGALE)){
        if(c2->type == AST_BINARY && (c2->binary.op == OP_ET || c2->binary.op == OP_OU)) 
            return 1;
        
        return -1;
    }
    // ET OU
    if(c1->type == AST_BINARY && c1->binary.op == OP_ET)
    {
        if(c2->type == AST_BINARY && c2->binary.op == OP_OU)
            return 1;

        return -1;
    }
    if(c1->type == AST_BINARY && c1->binary.op == OP_OU)
        return -1;
}

// write to language
ast_list_t *reverseList(ast_list_t* head)
{
    ast_list_t *prevNode, *curNode;

    if (head != NULL)
    {
        prevNode = head;
        curNode = head->next;
        head = head->next;

        prevNode->next = NULL; // Make first node as last node

        while (head != NULL)
        {
            head = head->next;
            curNode->next = prevNode;

            prevNode = curNode;
            curNode = head;
        }

        return prevNode; // Make last node as head

    }
}
char *mapper(char *key)
{
    char* res = NULL;
    if (strcmp(key, "entier") == 0)
    {
        res = (char *)malloc((strlen("int") + 1) * sizeof(char));
        strcpy(res, "int");
    }
    else if (strcmp(key, "rien") == 0)
    {
        res = (char *)malloc((strlen("void") + 1) * sizeof(char));
        strcpy(res, "void");
    }
    else if (strcmp(key, "si") == 0)
    {
        res = (char *)malloc((strlen("if") + 1) * sizeof(char));
        strcpy(res, "if");
    }
    else if (strcmp(key, "sinon") == 0)
    {
        res = (char *)malloc((strlen("else") + 1) * sizeof(char));
        strcpy(res, "else");
    }
    else if (strcmp(key, "tantque") == 0)
    {
        res = (char *)malloc((strlen("while") + 1) * sizeof(char));
        strcpy(res, "while");
    }
    else if (strcmp(key, "retourner") == 0)
    {
        res = (char *)malloc((strlen("return") + 1) * sizeof(char));
        strcpy(res, "return");
    }

    return res;
    
}

void writer(ast_list_t* ast)
{
    FILE *fp = fopen("main.in.c", "w+");
    char *writestr = (char *)malloc(475*sizeof(char));

    ast = reverseList(ast);
    while(ast)
    {
        strcat(writestr, write_function(ast->value));
        ast = ast->next;
    }
    // If file opened successfully, then write the string to file
    if ( fp )
    {
        fputs(writestr, fp);
        fflush(fp);
        fclose(fp);
    }
    else
    {
        printf("Failed to open the file\n");
    }
}

char* write_function(ast_t* fn)
{
    char *writer = NULL;
    int param_size = 3; // 3 for ( and ) and \0
    ast_list_t *param_list = reverseList(fn->function.params);
    ast_list_t *tmp = param_list;

    while (tmp)
    {
        param_size += strlen(mapper(typeToString(tmp->value->var.type))) + 1;
        param_size += strlen(tmp->value->var.name) ;
        tmp = tmp->next;
        if(tmp)
            param_size++;
    }
    bool first = true;
    char *params = (char *)malloc(475 * sizeof(char));
    tmp = param_list;
    while (tmp)
    {
        if (first)
            strcpy(params, "(");

        else
            strcat(params, ",");

        strcat(strcat(strcat(params, mapper(typeToString(tmp->value->var.type))), " "), tmp->value->var.name);
        tmp = tmp->next;
        first = false;
        if(!tmp)
            strcat(params, ")");
    }
    int signsize = strlen(mapper(typeToString(fn->function.return_type))) +
                   1 + strlen(fn->function.name) + param_size;
    writer = (char *)malloc(475 * sizeof(char));
    strcat(strcat(strcat(strcpy(writer, mapper(typeToString(fn->function.return_type))), " "),
                  fn->function.name),
           (param_size != 3) ? params : "()");
    
    return strcat(writer, write_block_content(fn->function.stmts));
}

char *write_block_content(ast_list_t *content)
{
    content = reverseList(content);
    char *writer = (char *)malloc(475*sizeof(char));
    writer[0] = '\n';
    writer[1] = '{';
    writer[2] = '\0';
    int full_size = 3; // 3 for \n{ and \n} and \0

    ast_list_t *tmp = content;
    while (tmp)
    {
        char *res = (char *)malloc(475*sizeof(char));
        switch (tmp->value->type)
        {
            case AST_FNCALL:
                res = write_expression(tmp->value);
            break;
            case AST_CONDITION:
                res = write_condition(tmp->value);
                break;
            case AST_LOOP:
				break;
            case AST_DECLARATION:
                res = write_declaration(tmp->value->declaration.lvalue, tmp->value->declaration.rvalue); 
                break;
            case AST_ASSIGNMENT:
                res = write_assignment(tmp->value->assignment.lvalue, tmp->value->assignment.rvalue);
                break;
            case AST_COMPOUND_STATEMENT:
				break;
            case AST_RETURN:;
                res = strcat(strcat(strcat(res, "return "), write_expression(tmp->value->ret.expr)), ";");
                break;
            default : 
                break;
        }
        if(res)
        {
            writer = (char *)realloc(writer, 475* sizeof(char));

            strcat(writer, "\n");
            strcat(writer, res);
        }
        tmp = tmp->next;
        if (tmp)
            full_size++;
    }
    // bool first = true;
    // char *params = (char *)malloc(475 * sizeof(char));
    // tmp = content;
    // while (tmp)
    // {
    //     if (first)
    //         strcpy(params, "\n{");

    //     else
    //         strcat(params, ";}");

    //     strcat(strcat(strcat(params, mapper(typeToString(tmp->value->var.type))), " "), tmp->value->var.name);
    //     tmp = tmp->next;
    //     first = false;
    //     if (!tmp)
    //         strcat(params, "\n}");
    // }
    // int signsize = strlen(mapper(typeToString(fn->function.return_type))) +
    //                1 + strlen(fn->function.name) + full_size;
    // writer = (char *)malloc((1 + strlen(fn->function.name)) * sizeof(char));
    // strcat(strcat(strcat(strcpy(writer, mapper(typeToString(fn->function.return_type))), " "),
    //               fn->function.name),
    //        (params) ? params : "()");

    strcat(writer, "\n}\n");
    return writer;
}

char *write_declaration(ast_t *lvalue, ast_t *rvalue)
{
    char *writer = NULL;
    int fullSize = 2; // \n and \0 and ' = ' and ;

    fullSize += strlen(mapper(typeToString(lvalue->var.type))) + strlen(lvalue->var.name);

    writer = (char*)malloc(475*sizeof(char));
    strcat(writer, mapper(typeToString(lvalue->var.type)));
    strcat(writer, " ");
    strcat(writer, write_assignment(lvalue, rvalue));

    return writer;
}

char *write_assignment(ast_t *lvalue, ast_t *rvalue)
{
    char *writer = NULL;
    int fullSize = 5; // \n and \0 and ' = ' and ;

    char *exp = NULL;
    if (rvalue)
        exp = write_expression(rvalue);
    fullSize += strlen(exp);

    writer = (char *)malloc(475 * sizeof(char));
    strcat(strcat(strcat(strcat(writer, lvalue->var.name), (rvalue) ? " = " : ""), exp), ";");

    return writer;
}

char* write_condition(ast_t* branch)
{
    char *writer = NULL;
    int fullSize = 2; // \n and \0 and ' = ' and ;

    fullSize += strlen(mapper("si"))+1;

    writer = (char *)malloc(475 * sizeof(char));
    char *block = write_block_content(branch->branch.valid->compound_stmt.stmts);
    strcat(writer, mapper("si"));
    strcat(writer, "(");
    strcat(writer, write_expression(branch->branch.condition));
    strcat(writer, ")");
    strcat(writer, block);
    if (branch->branch.invalid)
    {
        // char *sinon = write_condition(branch->branch.invalid);
        // strcat(writer, "sinon ");
        // strcat(writer, sinon);
    }

    return writer;
}

char *write_expression(ast_t *exp)
{
    char *str = NULL;
    switch (exp->type)
    {
    case AST_INTEGER:
        str = (char *)malloc(475 * sizeof(char));
        sprintf(str, "%ld", exp->integer);
        return str;
    case AST_VARIABLE:
        str = (char *)malloc((strlen(exp->var.name)+1) * sizeof(char));
        return strcat(str, exp->var.name);
    case AST_BINARY:
        str = (char *)malloc(475 * sizeof(char));
        return  strcat(strcat(strcat(str, write_expression(exp->binary.left)),
                binaryEnumToString(exp->binary.op)),
                write_expression(exp->binary.right));
    case AST_UNARY:
        str = (char *)malloc(475 * sizeof(char));
        return strcat(strcat(strcat(str, "("),
                    write_expression(exp->unary.operand)),
                    ")");
    case AST_FNCALL:;
        ast_list_t *tmp = reverseList(exp->call.args);
        char *params = (char *)malloc(475*sizeof(char));

        while (tmp)
        {
            if (strlen(params) == 0)
                strcat(params, "(");
            strcat(params, write_expression(tmp->value));
            tmp = tmp->next;
            if (strlen(params) > 1 && tmp)
                strcat(params, ",");
        }
        strcat(params, ")");

        int arg_size = strlen(params) + 1;
        str = (char *)malloc((strlen(exp->call.name) + arg_size + 3) * sizeof(char));
        strcat(strcat(str, exp->call.name),params);
        return str;
    case AST_RETURN:
        str = (char *)malloc(475 * sizeof(char));
        return strcat(strcat(str, "return "),
                    write_expression(exp->ret.expr));
    default:
        return "";
    }
}
