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
        default: fatal("DIGIT or OPERATOR");
        }
        stream++;
}


