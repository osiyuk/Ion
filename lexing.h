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
        TOKEN_FLOAT,
        TOKEN_NAME
} kind_t;

struct Token {
        kind_t kind;
        union {
                uint64_t val;
                double float_val;
                struct {
                        const char *start;
                        size_t length;
                };
                const char *name;
        };
};

struct Token token;
const char *stream;


void scan_int()
{
        char digit;
        uint64_t val = 0;
        
        while (isdigit(*stream)) {
                digit = (*stream - '0');
                
                val = val * 10 + digit;
                stream++;
        }
        token.val = val;
}


void scan_float()
{
        double val;
        const char *s = stream;
        
        while (isdigit(*s)) s++;
        if (*s == '.') s++;
        while (isdigit(*s)) s++;
        if (tolower(*s) == 'e') {
                s++;
                if (*s == '+' || *s == '-') s++;
                if (!isdigit(*s))
                        syntax_error("exponent has no digits");
                while (isdigit(*s)) s++;
        }
        
        val = strtod(stream, NULL);
        token.float_val = val;
        stream = s;
}


void next_token()
{
        char c;
        const char *str;
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
                str = stream;
                while (isdigit(*str))
                        str++;
                
                c = *str;
                if (c == '.' || tolower(c) == 'e') {
                        token.kind = TOKEN_FLOAT;
                        scan_float();
                } else {
                        token.kind = TOKEN_INT;
                        scan_int();
                }
                return;
        case '.':
                token.kind = TOKEN_FLOAT;
                scan_float();
                return;
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
#define assert_token_float(x) assert(token.float_val == (x) && match_token(TOKEN_FLOAT))
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


void lex_float_literal_tests()
{
        stream = "3.1415 .318 1e0 1. 13E200";
        next_token();
        assert_token_float(3.1415);
        assert_token_float(.318);
        assert_token_float(1e0);
        assert_token_float(1.);
        assert_token_float(13E200);
        assert_token_eof();
}


void lex_test()
{
        lex_integer_literal_tests();
        lex_float_literal_tests();
}

#endif

