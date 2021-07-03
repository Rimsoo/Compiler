#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "lexer.h"

ast_t* analyse_fonction()
{
    ast_t* res;

    return res;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Add file to compile in parameter\n");
        return EXIT_FAILURE;
    }
    
    FILE* fd = fopen(argv[1], "r");
    buffer_t* buffer = (buffer_t*)malloc(sizeof(buffer_t));
    buf_init(buffer, fd);

    while ( !buf_eof(buffer) )
    {
        if(strcmp(lexer_getalphanum(buffer), "fonction") != 0)
            syntax_error("Function expected");
        else
        {
            analyse_fonction
        }
    }
    

    return EXIT_SUCCESS;
}