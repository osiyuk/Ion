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
        default: fatal("expected VALID token, got '%c'", *stream);
        }
        stream++;
}


// recursive descent parser, based on EBNF grammar

// unary = ['-' | '~'] INT
// factor = unary {('*' | '/' | '%' | '<<' | '>>' | '&') unary}
// term = factor {('+' | '-' | '|' | '^') factor}

void parse_unary()
{
        puts("parse_unary()");
        
        if (token == OP_SUB || token == OP_TILDE) {
                switch (token) {
                case OP_SUB: puts("PARSED OP_SUB"); break;
                case OP_TILDE: puts("PARSED OP_TILDE"); break;
                }
                next();
        }
        
        if (token != INT)
                fatal("expected INT token, got '%c'", *stream);
        
        printf("PARSED INT %d\n", val);
        next();
}


void parse_factor()
{
        puts("parse_factor()");
        
        parse_unary();
        
        if (OP_MUL <= token && token <= OP_AND) {
                switch (token) {
                case OP_MUL: puts("PARSED OP_MUL"); break;
                case OP_DIV: puts("PARSED OP_DIV"); break;
                case OP_MOD: puts("PARSED OP_MOD"); break;
                case OP_LSHIFT: puts("PARSED OP_LSHIFT"); break;
                case OP_RSHIFT: puts("PARSED OP_RSHIFT"); break;
                case OP_AND: puts("PARSED OP_AND"); break;
                }
                next();
                parse_unary();
        }
}


void parse_term()
{
        puts("parse_term()");
        
        parse_factor();
        
        if (OP_ADD <= token && token <= OP_XOR) {
                switch (token) {
                case OP_ADD: puts("PARSED OP_ADD"); break;
                case OP_SUB: puts("PARSED OP_SUB"); break;
                case OP_OR: puts("PARSED OP_OR"); break;
                case OP_XOR: puts("PARSED OP_XOR"); break;
                }
                next();
                parse_factor();
        }
}


void parse_expr(const char *str)
{
        stream = str;
        next();
        parse_term();
}


int main(int argc, char *argv[])
{
        parse_expr("12*34 + -45/56 + ~25");
        return 0;
}

