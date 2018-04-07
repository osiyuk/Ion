#ifndef ION_LEXING
#define ION_LEXING

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "error_reporting.h"
#include "string_interning.h"

typedef enum {
        TOKEN_EOF,
        TOKEN_INT = 128,
        TOKEN_NAME
} kind_t;

struct Token {
        kind_t kind;
        union {
                uint64_t val;
                struct {
                        const char *start;
                        size_t length;
                };
                const char *name;
        };
};

struct Token token;
const char *stream;


void next_token()
{
repeat:
        switch (*stream) {
        case 0:
                token.kind = TOKEN_EOF;
                return;
        case ' ':
                while (isspace(*stream))
                        stream++;
                goto repeat;
        case '0'...'9':
                token.kind = TOKEN_INT;
                token.val = 0;
                while (isdigit(*stream)) {
                        token.val *= 10;
                        token.val += (*stream - '0');
                        stream++;
                }
                break;
        case 'a'...'z':
        case 'A'...'Z':
        case '_':
                token.kind = TOKEN_NAME;
                token.start = stream++;
                while (isalnum(*stream)
                        || *stream == '_') stream++;
                token.length = stream - token.start;
                token.name = str_intern_slice(token.start, token.length);
                break;
        default:
                syntax_error("Invalid '%c' token", *stream);
                stream++;
        }
}


void print_token()
{
        switch (token.kind) {
        case TOKEN_INT:
                printf("TOKEN_INT = %lu\n", token.val);
                break;
        case TOKEN_NAME:
                printf("TOKEN_NAME : %.*s\n", (int) token.length, token.start);
                break;
        default:
                printf( isprint(token.kind) ?
                        "TOKEN '%c'\n" :
                        "TOKEN %d\n", token.kind);
        }
}


char match_token(uint8_t kind)
{
        if (token.kind == kind) {
                next_token();
                return 1;
        }
        return 0;
}


#define assert_token_int(x) assert(token.val == (x) && match_token(TOKEN_INT))
#define assert_token_eof() assert(token.kind == TOKEN_EOF)


void lex_integer_literal_tests()
{
        stream = "0 1 2147990990 18446744073709551615";
        next_token();
        assert_token_int(0);
        assert_token_int(1);
        assert_token_int(2147990990);
        assert_token_int(18446744073709551615u);
        assert_token_eof();
}


void lex_test()
{
        lex_integer_literal_tests();
}

#endif

