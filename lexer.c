#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

void syntax_error(char* error)
{
    printf("Syntax error : %s\n", error);
    exit(EXIT_FAILURE);
}

char * lexer_getuntil (buffer_t * buffer, char c)
{
    char* res = NULL;
    int size = 0;

    buf_skipblank(buffer);
    buf_lock(buffer);
    while (buf_getchar(buffer) != c)
        size++;

    if(size)
    {
        buf_rollback(buffer, size);
        res = (char*) malloc((size+1) * sizeof(char));
        buf_getnchar(buffer, res, size);
        res[size] = '\0';
    }
    else
        res = "";

    buf_rollback(buffer, 1);
    buf_unlock(buffer);

    return res;
}

char * lexer_getuntil_rollback (buffer_t * buffer, char c)
{
    char* res = lexer_getuntil(buffer, c);
    buf_rollback(buffer, strlen(res)+1);

    return res;
}

char * lexer_getalphanum (buffer_t * buffer)
{
    char* res = NULL;
    int size = 0;
    bool wasLocked = buffer->islocked;
    buf_skipblank(buffer);
    if (!wasLocked) 
        buf_lock(buffer);
    while (isalnum(buf_getchar(buffer)))
        size++;

    if(size)
    {
        buf_rollback(buffer, size);
        res = (char*) malloc((size+1) * sizeof(char));
        buf_getnchar(buffer, res, size);
        res[size] = '\0';
    }
    else
        res = "";

    buf_rollback(buffer, 1);
    if (!wasLocked)
        buf_unlock(buffer);

    return res;
}

char * lexer_getalphanum_rollback (buffer_t * buffer)
{    
    buf_skipblank(buffer);
    buf_lock(buffer);
    char* res = lexer_getalphanum(buffer);
    buf_rollback(buffer, strlen(res)+1);
    buf_unlock(buffer);

    return res;
}

long lexer_getnumber (buffer_t * buffer)
{
    char* tmp = NULL;
    int size = 0;
    long res = 0;

    buf_skipblank(buffer);
    buf_lock(buffer);
    if (buf_getchar_rollback(buffer) == '-')
    {
        size ++;
        buf_forward(buffer, 1);
    }
    
    while (isdigit(buf_getchar(buffer)))
        size++;
    
    if(size)
    {
        buf_rollback(buffer, size);
        tmp = (char*) malloc((size+1) * sizeof(char));
        buf_getnchar(buffer, tmp, size);

        char** end = (char**)malloc(sizeof(char*));
        res = strtol(tmp, end, 10);

        if (*end == tmp)
            syntax_error("Bad number");
    }
    else
        syntax_error("Number expected !");

    buf_rollback(buffer, 1);
    buf_unlock(buffer);

    return res;
}

void lexer_assert_simplechar (buffer_t *buffer, char chr, char *msg)
{
    if (buf_getchar(buffer) != chr)
        syntax_error(msg);
}

void lexer_assert_twopoints (buffer_t *buffer, char *msg)
{
    lexer_assert_simplechar(buffer, ':', msg);
}

void lexer_assert_newline (buffer_t *buffer, char *msg)
{
    lexer_assert_simplechar(buffer, '\n', msg);
}

void lexer_assert_semicolon (buffer_t *buffer, char *msg)
{
    lexer_assert_simplechar(buffer, ';', msg);
}

void lexer_assert_openbrace (buffer_t *buffer, char *msg)
{
    lexer_assert_simplechar(buffer, '(', msg);
}

void lexer_assert_closebrace (buffer_t *buffer, char *msg)
{
    lexer_assert_simplechar(buffer, ')', msg);
}

void lexer_assert_openbracket (buffer_t *buffer, char *msg)
{
    lexer_assert_simplechar(buffer, '{', msg);
}

void lexer_assert_closebracket (buffer_t *buffer, char *msg)
{
    lexer_assert_simplechar(buffer, '}', msg);
}

void lexer_assert_blank (buffer_t *buffer, char *msg)
{
    if ( !ISBLANK(buf_getchar(buffer)) )
        syntax_error(msg);
}

char * lexer_getop(buffer_t *buffer)
{
    char* op;
    char c = buf_getchar_after_blank(buffer);
    
    if ( !isop(c) )
        syntax_error("Operator expected");
    
    char tmp = buf_getchar_rollback(buffer);
    if( isop(tmp) )
    {
        buf_forward(buffer, 1);
        char o[]= {c, tmp, '\0'};

        if(!isValidOp(o))
            syntax_error("Invalid Operator");

        op = (char*) malloc(3*sizeof(char));
        op[0] = c;
        op[1] = tmp;
        op[2] = '\0';
    }
    else
    {
        op = (char*) malloc(2*sizeof(char));
        op[0] = c;
        op[1] = '\0';
    }

    return op;
}

bool isValidOp(char* op)
{
    switch (strlen(op))
    {
    case 2:
        return  strcmp(op, ">=") == 0 ||
                strcmp(op, "<=") == 0 ||
                strcmp(op, "==") == 0 ||
                strcmp(op, "!=") == 0 ||
                strcmp(op, "ET") == 0 ||
                strcmp(op, "OU") == 0 ;
    case 1:
        return isop(op[1]);
    default:
        return false;
    }
    
}

bool isop (char chr)
{
    return chr == '=' || chr == '<' || chr == '>' || chr == ':' ||  chr == '+'
        || chr == '-' || chr == '*' || chr == '!' || chr == '/' 
        || chr == 'E' || chr == 'O' || chr == 'T' || chr == 'U';  
}

bool isNumber(char* s)
{
    int i = 0;
    if(strlen(s) > 1 && s[0] != '-')    i++;
    
    for (i = i; i < strlen(s); i++)
        if (!isdigit(s[i]))
            return false;  
    
    return true;
}