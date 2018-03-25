#include <ctype.h>
#include <stdio.h>

#include "fatal.h"

// expression lexer for arithmetic language
// composed of integer literals and operators

enum {
        INT, OP_TILDE,
        OP_MUL, OP_DIV, OP_MOD, OP_LSHIFT, OP_RSHIFT, OP_AND,
        OP_ADD, OP_SUB, OP_OR, OP_XOR
};

const char *stream;
int token, val;


void expect_char(char c)
{
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
                token = INT;
                val = 0;
                while (isdigit(*stream)) {
                        val *= 10;
                        val += (*stream++ - '0');
                }
                return;
        case '~': token = OP_TILDE; break;
        case '*': token = OP_MUL; break;
        case '/': token = OP_DIV; break;
        case '%': token = OP_MOD; break;
        case '<':
                stream++;
                expect_char('<');
                token = OP_LSHIFT;
                break;
        case '>':
                stream++;
                expect_char('>');
                token = OP_RSHIFT;
                break;
        case '&': token = OP_AND; break;
        case '+': token = OP_ADD; break;
        case '-': token = OP_SUB; break;
        case '|': token = OP_OR; break;
        case '^': token = OP_XOR; break;
        
        case 0: return;
        default: fatal("expected VALID token, got '%c'", *stream);
        }
        stream++;
}


void print_token()
{
        switch (token) {
        case INT:       printf("INT %d\n", val); break;
        case OP_TILDE:  puts("OP_TILDE"); break;
        case OP_MUL:    puts("OP_MUL"); break;
        case OP_DIV:    puts("OP_MUL"); break;
        case OP_MOD:    puts("OP_MOD"); break;
        case OP_LSHIFT: puts("OP_LSHIFT"); break;
        case OP_RSHIFT: puts("OP_RSHIFT"); break;
        case OP_AND:    puts("OP_AND"); break;
        case OP_ADD:    puts("OP_ADD"); break;
        case OP_SUB:    puts("OP_SUB"); break;
        case OP_OR:     puts("OP_OR"); break;
        case OP_XOR:    puts("OP_XOR"); break;
        }
}


// recursive descent parser, based on EBNF grammar

// unary = ['-' | '~'] INT
// factor = unary {('*' | '/' | '%' | '<<' | '>>' | '&') unary}
// term = factor {('+' | '-' | '|' | '^') factor}

void parse_unary()
{
        if (token == OP_SUB || token == OP_TILDE) {
                print_token();
                next();
        }
        
        if (token != INT)
                fatal("expected INT token, got '%c'", *stream);
        
        print_token();
        next();
        return;
}


void parse_factor()
{
        parse_unary();
        
        while (OP_MUL <= token && token <= OP_AND) {
                print_token();
                next();
                parse_unary();
        }
}


void parse_term()
{
        parse_factor();
        
        while (OP_ADD <= token && token <= OP_XOR) {
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

