#ifndef ION_LEXING
#define ION_LEXING

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "error_reporting.h"
#include "string_interning.h"

typedef enum {
        TOKEN_EOF,
        TOKEN_L_PAREN,
        TOKEN_R_PAREN,
        TOKEN_L_BRACE,
        TOKEN_R_BRACE,
        TOKEN_L_BRACKET,
        TOKEN_R_BRACKET,
        TOKEN_QUESTION,
        TOKEN_COLON,
        TOKEN_SEMICOLON,
        TOKEN_COMMA,
        TOKEN_DOT,
        TOKEN_NOT,
        TOKEN_NEG,
        TOKEN_KEYWORD,
        TOKEN_NAME,
        TOKEN_INT,
        TOKEN_FLOAT,
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


uint8_t char_to_digit[] = {
        ['0'] = 0,
        ['1'] = 1,
        ['2'] = 2,
        ['3'] = 3,
        ['4'] = 4,
        ['5'] = 5,
        ['6'] = 6,
        ['7'] = 7,
        ['8'] = 8,
        ['9'] = 9,
        ['a'] = 10, ['A'] = 10,
        ['b'] = 11, ['B'] = 11,
        ['c'] = 12, ['C'] = 12,
        ['d'] = 13, ['D'] = 13,
        ['e'] = 14, ['E'] = 14,
        ['f'] = 15, ['F'] = 15,
};


void scan_int(char base)
{
        char digit;
        uint64_t val = 0;
        
        while (isxdigit(*stream)) {
                digit = char_to_digit[(int) *stream];
                
                if (digit >= base) {
                        syntax_error(
                                "invalid digit \"%d\" in octal constant",
                                digit);
                        digit = 0;
                }
                
                val = val * base + digit;
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
        char c, base;
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
        case '1'...'9':
                str = stream;
                while (isdigit(*str))
                        str++;
                
                c = *str;
                if (c == '.' || tolower(c) == 'e') {
                        token.kind = TOKEN_FLOAT;
                        scan_float();
                        return;
                }
                token.kind = TOKEN_INT;
                scan_int(10);
                return;
        case '0':
                base = 8;
                stream++;
                if (*stream == 'x') {
                        stream++;
                        base = 16;
                }
                token.kind = TOKEN_INT;
                scan_int(base);
                return;
        case '.':
                if (isdigit(stream[1])) {
                        token.kind = TOKEN_FLOAT;
                        scan_float();
                        return;
                }
                token.kind = TOKEN_DOT;
                stream++;
                return;
#define CASE(c, k) \
        case c: \
                token.kind = k; \
                stream++; \
                return;
        CASE('(', TOKEN_L_PAREN)
        CASE(')', TOKEN_R_PAREN)
        CASE('[', TOKEN_L_BRACE)
        CASE(']', TOKEN_R_BRACE)
        CASE('{', TOKEN_L_BRACKET)
        CASE('}', TOKEN_R_BRACKET)
        CASE('?', TOKEN_QUESTION)
        CASE(':', TOKEN_COLON)
        CASE(';', TOKEN_SEMICOLON)
        CASE(',', TOKEN_COMMA)
        CASE('!', TOKEN_NOT)
        CASE('~', TOKEN_NEG)
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


#define assert_token(kind) assert(match_token(kind))
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
        
        stream = "013 0xFF 0x00abcdef 0xffffffffffffffff";
        next_token();
        assert_token_int(013);
        assert_token_int(0xFF);
        assert_token_int(0x00abcdef);
        assert_token_int(0xffffffffffffffff);
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


void lex_basic_token_tests()
{
        stream = "()[]{}?:;, .!~";
        next_token();
        assert_token(TOKEN_L_PAREN);
        assert_token(TOKEN_R_PAREN);
        assert_token(TOKEN_L_BRACE);
        assert_token(TOKEN_R_BRACE);
        assert_token(TOKEN_L_BRACKET);
        assert_token(TOKEN_R_BRACKET);
        assert_token(TOKEN_QUESTION);
        assert_token(TOKEN_COLON);
        assert_token(TOKEN_SEMICOLON);
        assert_token(TOKEN_COMMA);
        assert_token(TOKEN_DOT);
        assert_token(TOKEN_NOT);
        assert_token(TOKEN_NEG);
        assert_token_eof();
}


void lex_test()
{
        lex_integer_literal_tests();
        lex_float_literal_tests();
        lex_basic_token_tests();
}

#endif

