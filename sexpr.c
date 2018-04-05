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
        END
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
struct token next;


void expect_char(char c)
{
        stream++;
        if (c == *stream)
                return;
        
        fatal("expected '%c', got '%c'", c, *stream);
}


void consume()
{
        while (*stream == ' ')
                stream++;
        
        switch (*stream) {
        case '0'...'9':
                next.type = INT;
                next.val = 0;
                while (isdigit(*stream)) {
                        next.val *= 10;
                        next.val += (*stream++ - '0');
                }
                return;
        case '-': // SUB
        case '~': // TILDE
                next.type = UNARY;
                goto operator;
        case '*':
        case '/':
        case '%':
        case '&':
                next.type = FACTOR;
                goto operator;
        case '<':
                expect_char('<');
                next.type = FACTOR;
                strcpy(next.op, "<<");
                goto stream;
        case '>':
                expect_char('>');
                next.type = FACTOR;
                strcpy(next.op, ">>");
                goto stream;
        case '+':
        case '|':
        case '^':
                next.type = TERM;
                goto operator;
        
        case 0:
                next.type = END;
                return;
        default: fatal("expected VALID token, got '%c'", *stream);
        }
operator:
        next.op[0] = *stream;
        next.op[1] = 0;
stream:
        stream++;
}


void print_token()
{
        switch (next.type) {
        case INT: printf("INT %d\n", next.val); break;
        default: printf("OP %s\n", next.op);
        }
}


// recursive descent parser, based on EBNF grammar

// unary = INT | ('-' | '~') INT
// factor = unary {('*' | '/' | '%' | '<<' | '>>' | '&') unary}
// term = factor {('+' | '-' | '|' | '^') factor}

void parse_int()
{
        if (next.type != INT)
                fatal("expexted INT token, got '%c'", *stream);
        
        print_token();
        consume();
}


void parse_unary()
{
        if (next.type == UNARY) {
                print_token();
                consume();
        }
        
        parse_int();
}


void parse_factor()
{
        parse_unary();
        
        while (next.type == FACTOR) {
                print_token();
                consume();
                parse_unary();
        }
}


void parse_term()
{
        parse_factor();
        
        while (next.type == TERM || *next.op == '-') {
                print_token();
                consume();
                parse_factor();
        }
}


void parse_expr(const char *str)
{
        stream = str;
        printf("PARSING %s\n", stream);
        consume();
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

