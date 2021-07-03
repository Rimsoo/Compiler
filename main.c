#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"

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

    parser(buffer);

    return EXIT_SUCCESS;
}