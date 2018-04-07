#ifndef ION_LEXING
#define ION_LEXING

#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "error_reporting.h"
#include "string_interning.h"

typedef enum {
        TOKEN_EOF,
        TOKEN_INC,
        TOKEN_DEC,
        
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
        TOKEN_STR,
        
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_MOD,
        TOKEN_AND,
        TOKEN_LSHIFT,
        TOKEN_RSHIFT,
        
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_XOR,
        TOKEN_OR,
        
        TOKEN_EQ,
        TOKEN_LT,
        TOKEN_GT,
        TOKEN_LTEQ,
        TOKEN_GTEQ,
        TOKEN_NEQ,
        TOKEN_LOGICAL_AND,
        TOKEN_LOGICAL_OR,
        
        TOKEN_ASSIGN,
        TOKEN_COLON_ASSIGN,
        TOKEN_MUL_ASSIGN,
        TOKEN_DIV_ASSIGN,
        TOKEN_MOD_ASSIGN,
        TOKEN_AND_ASSIGN,
        TOKEN_LSHIFT_ASSIGN,
        TOKEN_RSHIFT_ASSIGN,
        TOKEN_ADD_ASSIGN,
        TOKEN_SUB_ASSIGN,
        TOKEN_XOR_ASSIGN,
        TOKEN_OR_ASSIGN,
} kind_t;

struct Token {
        kind_t kind;
        union {
                uint64_t val;
                double float_val;
                const char *str_val;
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
                
                if (!isdigit(*s)) {
                        syntax_error("exponent has no digits");
                }
                while (isdigit(*s)) s++;
        }
        
        val = strtod(stream, NULL);
        token.float_val = val;
        stream = s;
}


void scan_str()
{
        char *str = NULL;
        char c;
        
        assert(*stream == '"');
        stream++;
        
        while (*stream && *stream != '"') {
                c = *stream;
                if (c == '\n') {
                        goto missing;
                }
                buf_push(str, c);
                stream++;
        }
        if (!*stream) {
missing:
                syntax_error("missing terminating \" character");
                token.str_val = NULL;
                return;
        }
        
        assert(*stream == '"');
        stream++;
        
        buf_push(str, 0);
        token.str_val = str;
}


void scan_char()
{
        char c = 0;
        
        assert(*stream == '\'');
        stream++;
        
        c = *stream;
        stream++;
        
        assert(*stream == '\'');
        stream++;
        
        token.val = c;
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
        case 10:
                while (isspace(*stream))
                        stream++;
                goto repeat;
        case '1'...'9':
                str = stream;
                while (isdigit(*str)) {
                        str++;
                }
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
        case 34: // "
                token.kind = TOKEN_STR;
                scan_str();
                return;
        case 39: // '
                token.kind = TOKEN_INT;
                scan_char();
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
#define CASE2(c, k, c1, k1) \
        case c: \
                token.kind = k; \
                stream++; \
                if (*stream == c1) { \
                        token.kind = k1; \
                        stream++;\
                } \
                return;
        CASE2('=', TOKEN_ASSIGN, '=', TOKEN_EQ)
        CASE2(':', TOKEN_COLON, '=', TOKEN_COLON_ASSIGN)
        CASE(';', TOKEN_SEMICOLON)
        CASE(',', TOKEN_COMMA)
        CASE2('!', TOKEN_NOT, '=', TOKEN_NEQ)
        CASE('~', TOKEN_NEG)
        CASE2('*', TOKEN_MUL, '=', TOKEN_MUL_ASSIGN)
        CASE2('/', TOKEN_DIV, '=', TOKEN_DIV_ASSIGN)
        CASE2('%', TOKEN_MOD, '=', TOKEN_MOD_ASSIGN)
#define CASE3(c, k, c1, k1, c2, k2) \
        case c: \
                token.kind = k; \
                stream++; \
                if (*stream == c1) { \
                        token.kind = k1; \
                        stream++; \
                } else if (*stream == c2) { \
                        token.kind = k2; \
                        stream++; \
                } \
                return;
        CASE3('&', TOKEN_AND, '=', TOKEN_AND_ASSIGN, '&', TOKEN_LOGICAL_AND)
        CASE3('+', TOKEN_ADD, '=', TOKEN_ADD_ASSIGN, '+', TOKEN_INC)
        CASE3('-', TOKEN_SUB, '=', TOKEN_SUB_ASSIGN, '-', TOKEN_DEC)
        CASE2('^', TOKEN_XOR, '=', TOKEN_XOR_ASSIGN)
        CASE3('|', TOKEN_OR, '=', TOKEN_OR_ASSIGN, '|', TOKEN_LOGICAL_OR)
#define CASE4(c, k, c1, k1, k2, k3) \
        case c: \
                token.kind = k; \
                stream++; \
                if (*stream == c1) { \
                        token.kind = k1; \
                        stream++; \
                } else if (*stream == c) { \
                        token.kind = k2; \
                        stream++; \
                        if (*stream == c1) { \
                                token.kind = k3; \
                                stream++; \
                        } \
                } \
                return;
        CASE4('<', TOKEN_LT, '=', TOKEN_LTEQ, TOKEN_LSHIFT, TOKEN_LSHIFT_ASSIGN)
        CASE4('>', TOKEN_GT, '=', TOKEN_GTEQ, TOKEN_RSHIFT, TOKEN_RSHIFT_ASSIGN)
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
#define assert_token_empty_str() assert(token.str_val == NULL && match_token(TOKEN_STR))
#define assert_token_str(x) assert(strcmp(token.str_val, (x)) == 0 && match_token(TOKEN_STR))
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


void lex_string_literal_tests()
{
        stream = "\"typedef\" \"enum\" \"struct\"";
        next_token();
        assert_token_str("typedef");
        assert_token_str("enum");
        assert_token_str("struct");
        assert_token_eof();
}


void lex_char_literal_tests()
{
        stream = "'a' 'b' 'c'";
        next_token();
        assert_token_int('a');
        assert_token_int('b');
        assert_token_int('c');
        assert_token_eof();
}


void lex_basic_token_tests()
{
        stream = "()[]{} ? : ; , . ! ~";
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


void lex_operator_tests()
{
        stream = "= := * / % & << >> + - ^ | == < > <= >= != && ||";
        next_token();
        assert_token(TOKEN_ASSIGN);
        assert_token(TOKEN_COLON_ASSIGN);
        assert_token(TOKEN_MUL);
        assert_token(TOKEN_DIV);
        assert_token(TOKEN_MOD);
        assert_token(TOKEN_AND);
        assert_token(TOKEN_LSHIFT);
        assert_token(TOKEN_RSHIFT);
        assert_token(TOKEN_ADD);
        assert_token(TOKEN_SUB);
        assert_token(TOKEN_XOR);
        assert_token(TOKEN_OR);
        assert_token(TOKEN_EQ);
        assert_token(TOKEN_LT);
        assert_token(TOKEN_GT);
        assert_token(TOKEN_LTEQ);
        assert_token(TOKEN_GTEQ);
        assert_token(TOKEN_NEQ);
        assert_token(TOKEN_LOGICAL_AND);
        assert_token(TOKEN_LOGICAL_OR);
        assert_token_eof();
        
        stream = "++ -- := *= /= %= &= <<= >>= += -= ^= |=";
        next_token();
        assert_token(TOKEN_INC);
        assert_token(TOKEN_DEC);
        assert_token(TOKEN_COLON_ASSIGN);
        assert_token(TOKEN_MUL_ASSIGN);
        assert_token(TOKEN_DIV_ASSIGN);
        assert_token(TOKEN_MOD_ASSIGN);
        assert_token(TOKEN_AND_ASSIGN);
        assert_token(TOKEN_LSHIFT_ASSIGN);
        assert_token(TOKEN_RSHIFT_ASSIGN);
        assert_token(TOKEN_ADD_ASSIGN);
        assert_token(TOKEN_SUB_ASSIGN);
        assert_token(TOKEN_XOR_ASSIGN);
        assert_token(TOKEN_OR_ASSIGN);
        assert_token_eof();
}


void lex_test()
{
        lex_integer_literal_tests();
        lex_float_literal_tests();
        lex_string_literal_tests();
        lex_char_literal_tests();
        lex_basic_token_tests();
        lex_operator_tests();
}

#endif

