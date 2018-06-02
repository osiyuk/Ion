#include "config.h"

#define MAIN dump_ast


void regression_tests(void)
{
        filename = "<anonymous>";
        buf_test();
        str_test();
        lex_test();
        parser_test();
}


char *read_file(const char *name)
{
        FILE *stream = NULL;
        char *content = NULL;
        int c;

        stream = fopen(name, "r");
        if (stream == NULL) goto error;

        while (c = fgetc(stream), c != EOF) {
                if (errno) goto error;
                buf_push(content, c);
        }
        if (errno) goto error;
        return content;
error:
        perror("read_file");
        return NULL;
}


int dump_ast(int argc, char **argv)
{
        Decl **ast = NULL;
        const char *name, *content;

        if (argc < 2)
                name = "example.ion";
        else    name = argv[1];

        content = read_file(name);
        if (content == NULL)
                return 1;

        init_lex(name, content);
        ast = recursive_descent_parser();

        print_ast(ast, buf_len(ast));
        return 0;
}


int main(int argc, char **argv)
{
        regression_tests();
        return MAIN(argc, argv);
}

