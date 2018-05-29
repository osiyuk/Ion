#include "config.h"

#define MAIN regression_tests


int regression_tests(int argc, char **argv)
{
        buf_test();
        str_test();
        lex_test();
        parser_test();
        return 0;
}


int main(int argc, char **argv)
{
        return MAIN(argc, argv);
}

