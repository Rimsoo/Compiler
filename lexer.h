#ifndef LEXER_H
#define LEXER_H
#include <stdbool.h>
#include "buffer.h"

char * lexer_getuntil (buffer_t * buffer, char c);
char * lexer_getuntil_rollback (buffer_t * buffer, char c);
char * lexer_getalphanum (buffer_t * buffer);
char * lexer_getalphanum_rollback (buffer_t * buffer);
long lexer_getnumber (buffer_t * buffer);

// control char
void lexer_assert_simplechar (buffer_t *buffer, char chr, char *msg);
void lexer_assert_twopoints (buffer_t *buffer, char *msg);
void lexer_assert_newline (buffer_t *buffer, char *msg);
void lexer_assert_semicolon (buffer_t *buffer, char *msg);
void lexer_assert_openbrace (buffer_t *buffer, char *msg);
void lexer_assert_closebrace (buffer_t *buffer, char *msg);
void lexer_assert_openbracket (buffer_t *buffer, char *msg);
void lexer_assert_closebracket (buffer_t *buffer, char *msg);
void lexer_assert_equalsign (buffer_t *buffer, char *msg);
void lexer_assert_blank (buffer_t *buffer, char *msg);

// operators
char * lexer_getop(buffer_t *buffer);
bool isop (char chr);
bool isnumber(char* s);
bool isValidOp(char* op);

void syntax_error(char* error);

#endif