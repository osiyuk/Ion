#ifndef AST_PRINT_AUTOMATON
#define AST_PRINT_AUTOMATON

#include <stdio.h>
#include "ast.h"


void print_expr(Expr *expr)
{
        switch (expr->kind) {
        case EXPR_NAME:
                printf(expr->name);
                return;
        case EXPR_INT:
                printf("%d", expr->int_val);
                return;
        case EXPR_FLOAT:
                printf("%f", expr->float_val);
                return;
        case EXPR_STR:
                printf(expr->str_val);
                return;
        default:
                assert(EXPR_NONE);
        }
}

#endif

