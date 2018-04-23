#include "stretchy_buffer.h"
#include "string_interning.h"
#include "lexer.h"
#include "ast.h"

int main(int argc, char **argv)
{
        buf_test();
        str_test();
        lex_test();
        return 0;
}
