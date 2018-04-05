#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "stretchy_buffer.h"
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
};


typedef struct tree_s tree_t;

struct tree_s {
        struct token token;
        tree_t *lval;
        tree_t *rval;
};


const char *stream;
struct token next;
tree_t *tree = NULL;


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
        case '-':
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


tree_t *push(tree_t node)
{
        int n = buf_len(tree);
        buf_push(tree, node);
        return tree + n;
}

#define leaf(token)             push((tree_t) {token, 0, 0})
#define node(op, lval, rval)    push((tree_t) {op, lval, rval})


// recursive descent parser, based on EBNF grammar

// int = INT
// unary = int | ('-' | '~') int
// factor = unary {('*' | '/' | '%' | '<<' | '>>' | '&') unary}
// term = factor {('+' | '-' | '|' | '^') factor}

tree_t *parse_int()
{
        tree_t *t;
        
        if (next.type != INT)
                fatal("expected INT token, got '%c'", *stream);
        
        t = leaf(next);
        consume();
        return t;
}


tree_t *parse_unary()
{
        tree_t *t;
        int is_unary;
        
        is_unary = (next.type == UNARY ||
                (next.type == TERM && *next.op == '-'));
        
        if (!is_unary)
                return parse_int();
        
        next.type = UNARY;
        t = leaf(next);
        consume();
        
        t->rval = parse_int();
        return t;
}


tree_t *parse_factor()
{
        tree_t *lval, *rval;
        struct token op;
        
        lval = parse_unary();
        
        while (next.type == FACTOR) {
                op = next;
                consume();
                rval = parse_unary();
                lval = node(op, lval, rval);
        }
        
        return lval;
}


tree_t *parse_term()
{
        tree_t *lval, *rval;
        struct token op;
        
        lval = parse_factor();
        
        while (next.type == TERM) {
                op = next;
                consume();
                rval = parse_factor();
                lval = node(op, lval, rval);
        }
        
        return lval;
}


void print_tree(tree_t *head)
{
        struct token tkn = head->token;
        
        switch (tkn.type) {
        case INT:
                printf("%d", tkn.val);
                break;
        case UNARY:
                head = head->rval;
                printf("(%s %d)", tkn.op, head->token.val);
                break;
        case FACTOR:
        case TERM:
                printf("(%s ", tkn.op);
                print_tree(head->lval);
                printf(" ");
                print_tree(head->rval);
                printf(")");
                break;
        default:
                printf("\nInvalid token type: %d\n", tkn.type);
        }
}


void parse_expr(const char *str)
{
        tree_t *t;
        buf_init(tree);
        
        stream = str;
        printf("PARSING %s\n", stream);
        consume();
        
        t = parse_term();
        print_tree(t);
        printf("\n");
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

