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


void print_token()
{
        switch (next.type) {
        case INT: printf("INT %d\n", next.val); break;
        default: printf("OP %s\n", next.op);
        }
}


tree_t *tree_push(tree_t node)
{
        int n = buf_len(tree);
        buf_push(tree, node);
        return tree + n;
}

#define push(token)             tree_push((tree_t) {token, 0, 0})
#define node(op, lval, rval)    tree_push((tree_t) {op, lval, rval})


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
        
        t = push(next);
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
        t = push(next);
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


void recursive_descent()
{
        tree_t *ast;
        consume();
        ast = parse_term();
        printf("RD: ");
        print_tree(ast);
        printf("\n");
}


// Shunting Yard algorithm

// expr = production {binary production}
// production = [unary] INT
// binary = '+' | '-' | '*' | '/' | '%' | '<<' | '>>' | '&' | '|' | '^'
// unary = '-' | '~'

tree_t **operators = NULL;
tree_t **operands = NULL;

void pop_operator()
{
        tree_t *op;
        
        op = buf_pop(operators);
        op->rval = buf_pop(operands);
        
        if (op->token.type != UNARY)
                op->lval = buf_pop(operands);
        buf_push(operands, op);
}


void push_operator(struct token op)
{
        struct token t;
        
check_precedence:
        if (buf_len(operators) == 0)
                goto push;
        
        t = buf_top(operators)->token;
        
        if (t.type <= op.type) {
                pop_operator();
                goto check_precedence;
        }
push:
        buf_push(operators, push(op));
}


void shunting_yard()
{
        tree_t *ast;
        int is_unary, is_binary;
        
        buf_init(operators);
        buf_init(operands);
        consume();
        
production:
        is_unary = (next.type == UNARY ||
                (next.type == TERM && *next.op == '-'));
        
        if (is_unary) {
                next.type = UNARY;
                push_operator(next);
                consume();
        }
        
        if (next.type != INT)
                fatal("expected INT token, got '%c'", *stream);
        
        buf_push(operands, push(next));
        consume();
        
        is_binary = (next.type == FACTOR || next.type == TERM);
        
        if (is_binary) {
                push_operator(next);
                consume();
                goto production;
        }
        
        while (buf_len(operators))
                pop_operator();
        
        if (buf_len(operands) != 1)
                fatal("shunting yard invariant broken, len(operands) != 1");
        
        ast = buf_pop(operands);
        printf("SY: ");
        print_tree(ast);
        printf("\n");
}


void parse_expr(const char *str)
{
        buf_init(tree);
        
        printf("PARSING %s\n", str);
        stream = str;
        recursive_descent();
        
        stream = str;
        shunting_yard();
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

