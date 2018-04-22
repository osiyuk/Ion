#ifndef ION_LEXING
#define ION_LEXING

#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "error_reporting.h"
#include "string_interning.h"

#include "tokens.h"

void init_keywords();
void next_token();


struct Token {
        TokenKind kind;
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


#define KEYWORD(name) const char *name##_keyword;
#include "keywords.txt"
#undef KEYWORD

const char **keywords;


void init_keywords()
{
        static char inited;
        if (inited) {
                return;
        }
        keywords = buf_grow(NULL, 32, sizeof(char *));
#define KEYWORD(name) KEYWORD1(name##_keyword, #name)
#define KEYWORD1(v, name) \
        v = str_intern(name); buf__push(keywords, v);
#include "keywords.txt"
#undef KEYWORD
        assert(str_intern("func") == func_keyword);
        inited = 1;
}


char is_keyword(const char *name)
{
        for (int i = 0; i < buf_len(keywords); i++) {
                if (name == keywords[i])
                        return 1;
        }
        return 0;
}


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

uint8_t escaped_char[] = {
        ['0'] = 0,
        ['"'] = '"',
        ['\''] = '\'',
        ['\\'] = '\\',
        ['n'] = '\n',
        ['r'] = '\r',
        ['t'] = '\t',
        ['v'] = '\v',
};


#define invalid_digit "invalid digit \"%d\" in octal constant"
#define unknown_escape "unknown escape sequence '\\%c'"
#define missing_term "missing terminating %c character"
#define unknown_token "unknown token '%c' %d, skipping"


void scan_int(char base)
{
        char digit;
        uint64_t val = 0;
        
        while (isxdigit(*stream)) {
                digit = char_to_digit[(int) *stream];
                
                if (digit >= base) {
                        syntax_error(invalid_digit, digit);
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
                switch (c) {
                case '\\':
                        stream++;
                        c = escaped_char[(int) *stream];
                        if (c == 0 && *stream != '0') {
                                syntax_error(unknown_escape, *stream);
                        }
                        break;
                case '\n':
                        goto missing;
                }
                buf_push(str, c);
                stream++;
        }
        if (!*stream) {
missing:
                syntax_error(missing_term, '"');
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
        
        switch (*stream) {
        case '\\':
                stream++;
                c = escaped_char[(int) *stream];
                if (c == 0 && *stream != '0') {
                        syntax_error(unknown_escape, *stream);
                }
                stream++;
                break;
        case '\'':
        case '\n':
                syntax_error(missing_term, '\'');
                stream++;
                token.val = 0;
                return;
        default:
                c = *stream;
                stream++;
        }
        
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
                str = stream++;
                while (isalnum(*stream) || *stream == '_') {
                        stream++;
                }
                
                token.name = str_intern_slice(str, stream - str);
                token.kind = is_keyword(token.name) ? TOKEN_KEYWORD : TOKEN_NAME;
                break;
        default:
                c = *stream;
                syntax_error(unknown_token, c, c);
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
#define assert_token_name(x) assert(strcmp(token.name, (x)) == 0 && match_token(TOKEN_NAME))
#define assert_token_keyword(x) assert(token.name == (x) && match_token(TOKEN_KEYWORD))
#define assert_token_eof() assert(token.kind == TOKEN_EOF)

#define init_stream(s) stream = s; next_token()


void lex_integer_literal_tests()
{
        init_stream("0 1 2147990990 18446744073709551615");
        assert_token_int(0);
        assert_token_int(1);
        assert_token_int(2147990990);
        assert_token_int(18446744073709551615u);
        assert_token_eof();
        
        init_stream("013 0xFF 0x00abcdef 0xffffffffffffffff");
        assert_token_int(013);
        assert_token_int(0xFF);
        assert_token_int(0x00abcdef);
        assert_token_int(0xffffffffffffffff);
        assert_token_eof();
}


void lex_float_literal_tests()
{
        init_stream("3.1415 .318 1e0 1. 13E200");
        assert_token_float(3.1415);
        assert_token_float(.318);
        assert_token_float(1e0);
        assert_token_float(1.);
        assert_token_float(13E200);
        assert_token_eof();
}


void lex_string_literal_tests()
{
        init_stream("\"typedef\" \"enum\" \"struct\"");
        assert_token_str("typedef");
        assert_token_str("enum");
        assert_token_str("struct");
        assert_token_eof();
        
        init_stream("\"escaped chars\\n\\r\\tquotes \\'\\\"\\\\ null\\0\"");
        assert_token_str("escaped chars\n\r\tquotes \'\"\\ null\0");
        assert_token_eof();
}


void lex_char_literal_tests()
{
        init_stream("'a' 'b' 'c'");
        assert_token_int('a');
        assert_token_int('b');
        assert_token_int('c');
        assert_token_eof();
        
        init_stream("'\\0' '\\n' '\\r' '\\t' '\\v' '\\'' '\\\"' '\\\\'");
        assert_token_int(0);
        assert_token_int('\n');
        assert_token_int('\r');
        assert_token_int('\t');
        assert_token_int('\v');
        assert_token_int('\'');
        assert_token_int('\"');
        assert_token_int('\\');
        assert_token_eof();
}


void lex_basic_token_tests()
{
        init_stream("()[]{} ? : ; , . ! ~");
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
        init_stream("= := * / % & << >> + - ^ | == < > <= >= != && ||");
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
        
        init_stream("++ -- := *= /= %= &= <<= >>= += -= ^= |=");
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


void lex_keyword_tests()
{
        init_stream("while my_name cast with const qualifier default kiril27");
        assert_token_keyword(while_keyword);
        assert_token_name("my_name");
        assert_token_keyword(cast_keyword);
        assert_token_name("with");
        assert_token_keyword(const_keyword);
        assert_token_name("qualifier");
        assert_token_keyword(default_keyword);
        assert_token_name("kiril27");
        assert_token_eof();
}


void lex_test()
{
        init_keywords();
        
        lex_integer_literal_tests();
        lex_float_literal_tests();
        lex_string_literal_tests();
        lex_char_literal_tests();
        lex_basic_token_tests();
        lex_operator_tests();
        lex_keyword_tests();
}

#endif

