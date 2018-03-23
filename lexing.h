#ifndef ION_LEXING
#define ION_LEXING

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "string_interning.h"

typedef enum {
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
        switch (*stream) {
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
                token.kind = *stream++;
        }
}


void print_token()
{
        switch (token.kind) {
        case TOKEN_INT:
                printf("TOKEN_INT = %lu\n", token.val);
                break;
        case TOKEN_NAME:
                printf("TOKEN_NAME : %.*s\n", token.length, token.start);
                break;
        default:
                printf( isprint(token.kind) ?
                        "TOKEN '%c'\n" :
                        "TOKEN %d\n", token.kind);
        }
}

void lex_test()
{
        stream = "var max_exprs = 1024\n\nconst PI = 3.14";
        while (next_token(), token.kind) {
                print_token();
        }
}

#endif

