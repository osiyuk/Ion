#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "fatal.h"

// expression lexer for arithmetic language
// composed of integer literals and operators

enum {
        NONE,
        INT,
        UNARY,
        FACTOR,
        TERM,
        
        LSHIFT = 128,
        RSHIFT,
};


struct token {
        u_char type;
        union {
                int val;
                char op[4];
        };
        struct token *lval;
        struct token *rval;
};

const char *stream;
struct token token;


void expect_char(char c)
{
        stream++;
        if (c == *stream)
                return;
        
        fatal("expected '%c', got '%c'", c, *stream);
}


void next()
{
        while (*stream == ' ')
                stream++;
        
        switch (*stream) {
        case '0'...'9':
                token.type = INT;
                token.val = 0;
                while (isdigit(*stream)) {
                        token.val *= 10;
                        token.val += (*stream++ - '0');
                }
                return;
        case '-': // SUB
        case '~': // TILDE
                token.type = UNARY;
                goto operator;
        case '*':
        case '/':
        case '%':
        case '&':
                token.type = FACTOR;
                goto operator;
        case '<':
                expect_char('<');
                token.type = FACTOR;
                strcpy(token.op, "<<");
                goto stream;
        case '>':
                expect_char('>');
                token.type = FACTOR;
                strcpy(token.op, ">>");
                goto stream;
        case '+':
        case '|':
        case '^':
                token.type = TERM;
                goto operator;
        
        case 0: return;
        default: fatal("expected VALID token, got '%c'", *stream);
        }
operator:
        token.op[0] = *stream;
        token.op[1] = 0;
stream:
        stream++;
}


void print_token()
{
        switch (token.type) {
        case INT: printf("INT %d\n", token.val); break;
        default: printf("OP %s\n", token.op);
        }
}


// recursive descent parser, based on EBNF grammar

// unary = ['-' | '~'] INT
// factor = unary {('*' | '/' | '%' | '<<' | '>>' | '&') unary}
// term = factor {('+' | '-' | '|' | '^') factor}

void parse_unary()
{
        if (token.type == UNARY) {
                print_token();
                next();
        }
        
        if (token.type != INT)
                fatal("expected INT token, got '%c'", *stream);
        
        print_token();
        next();
        return;
}


void parse_factor()
{
        parse_unary();
        
        while (token.type == FACTOR) {
                print_token();
                next();
                parse_unary();
        }
}


void parse_term()
{
        parse_factor();
        
        while (token.type == TERM || *token.op == '-') {
                print_token();
                next();
                parse_factor();
        }
}


void parse_expr(const char *str)
{
        stream = str;
        printf("PARSING %s\n", stream);
        next();
        parse_term();
}


int main(int argc, char *argv[])
{
        if (argc < 2) {
                printf("usage: %s \"expr\" ...\n", argv[0]);
                return 1;
        }
        
        for (int i = 1; i < argc; i++) {
                parse_expr(argv[i]);
        }
        
        return 0;
}

