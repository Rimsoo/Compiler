#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "lexer.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Add file to compile in parameter");
        return EXIT_FAILURE;
    }
    
    FILE* fd = fopen(argv[1], "r");
    buffer_t* buff = (buffer_t*)malloc(sizeof(buffer_t));
    buf_init(buff, fd);

    printf("alnum rollback : %s\n", lexer_getalphanum_rollback(buff));
    buf_print(buff);
    printf("alnum : %s\n", lexer_getalphanum(buff));
    buf_print(buff);
    printf("digit : %ld\n", lexer_getnumber(buff));
    buf_print(buff);

    return EXIT_SUCCESS;
}