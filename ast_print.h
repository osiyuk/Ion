#ifndef AST_PRINT_AUTOMATON
#define AST_PRINT_AUTOMATON

#include <stdio.h>
#include "ast.h"
#include "stretchy_buffer.h"

#define buf_printf(...) (print_buf = buf_printf(print_buf, __VA_ARGS__))
#define printf(...) \
        use_print_buf ? (void) buf_printf(__VA_ARGS__) : printf(__VA_ARGS__)

char use_print_buf, *print_buf;


void print_expr(Expr *expr)
{
        const Expr e = *expr;
        
        switch (expr->kind) {
        case EXPR_NAME:
                printf(expr->name);
                return;
        case EXPR_INT:
                printf("%ld", expr->int_val);
                return;
        case EXPR_FLOAT:
                printf("%f", expr->float_val);
                return;
        case EXPR_STR:
                printf("%c%s%c", '"', expr->str_val, '"');
                return;
        case EXPR_CAST:
                assert(EXPR_CAST);
        case EXPR_CALL:
                printf("(");
                print_expr(e.call.expr);
                for (size_t i = 0; i < e.call.num_args; i++) {
                        printf(" ");
                        print_expr(e.call.args[i]);
                }
                printf(")");
                return;
        case EXPR_INDEX:
                printf("([] ");
                print_expr(e.index.oexpr);
                printf(" ");
                print_expr(e.index.iexpr);
                printf(")");
                return;
        case EXPR_FIELD:
                printf("(. ");
                print_expr(e.field.expr);
                printf(" ");
                printf(e.field.name);
                printf(")");
                return;
        case EXPR_UNARY:
                printf("(%s ", token_kind(e.unary.op));
                print_expr(e.unary.expr);
                printf(")");
                return;
        case EXPR_BINARY:
                printf("(%s ", token_kind(e.binary.op));
                print_expr(e.binary.left);
                printf(" ");
                print_expr(e.binary.right);
                printf(")");
                return;
        case EXPR_TERNARY:
                printf("(? ");
                print_expr(e.ternary.cond);
                printf(" ");
                print_expr(e.ternary.expr);
                printf(" ");
                print_expr(e.ternary.or_expr);
                printf(")");
                return;
        case EXPR_COMPOUND:
                assert(EXPR_COMPOUND == 0);
        case EXPR_SIZEOF:
                printf("(sizeof ");
                print_expr(expr->sizeof_expr);
                printf(")");
                return;
        case EXPR_SIZEOF_TYPE:
                assert(EXPR_SIZEOF_TYPE);
        default:
                assert(EXPR_NONE);
        }
}

#undef printf
#endif

