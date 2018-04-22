#ifndef ABSTRACT_SYNTAX_TREE
#define ABSTRACT_SYNTAX_TREE

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Expr Expr;
typedef struct Typespec Typespec;


void *ast_alloc(size_t size)
{
        void *ptr;
        
        assert(size != 0);
        ptr = malloc(size);
        memset(ptr, 0, size);
        return ptr;
}


enum ExprKind {
        EXPR_NONE,
        EXPR_NAME,
        EXPR_INT,
        EXPR_FLOAT,
        EXPR_STR,
        EXPR_CAST,
        EXPR_CALL,
        EXPR_INDEX,
        EXPR_FIELD,
        EXPR_UNARY,
        EXPR_BINARY,
        EXPR_TERNARY,
        EXPR_COMPOUND,
};

struct Expr {
        enum ExprKind kind;
        union {
                const char *name;
                int64_t int_val;
                double float_val;
                const char *str_val;
                
                struct {
                        Typespec *type;
                        Expr *expr;
                } cast;
                
                struct {
                        Expr *expr;
                        Expr **args;
                        size_t num_args;
                } call;
                
                struct {
                        Expr *lexpr;
                        Expr *pexpr;
                } index;
                
                struct {
                        Expr *expr;
                        const char *name;
                } field;
                
                struct {
                        TokenKind op;
                        Expr *expr;
                } unary;
                
                struct {
                        TokenKind op;
                        Expr *left;
                        Expr *right;
                } binary;
                
                struct {
                        Expr *cond;
                        Expr *expr;
                        Expr *or_expr;
                } ternary;
        };
};


Expr *new_expr(enum ExprKind kind)
{
        Expr *e = ast_alloc(sizeof(Expr));
        e->kind = kind;
        return e;
}


Expr *new_expr_name(const char *name)
{
        Expr *e = new_expr(EXPR_NAME);
        e->name = name;
        return e;
}


Expr *new_expr_int(int64_t int_val)
{
        Expr *e = new_expr(EXPR_INT);
        e->int_val = int_val;
        return e;
}


Expr *new_expr_float(double float_val)
{
        Expr *e = new_expr(EXPR_FLOAT);
        e->float_val = float_val;
        return e;
}


Expr *new_expr_str(const char *str_val)
{
        Expr *e = new_expr(EXPR_STR);
        e->str_val = str_val;
        return e;
}


Expr *new_expr_cast(Typespec *type, Expr *expr)
{
        Expr *e = new_expr(EXPR_CAST);
        e->cast.type = type;
        e->cast.expr = expr;
        return e;
}


Expr *new_expr_call(Expr *expr, Expr **args, size_t num_args)
{
        Expr *e = new_expr(EXPR_CALL);
        e->call.expr = expr;
        e->call.args = args;
        e->call.num_args = num_args;
        return e;
}


Expr *new_expr_index(Expr *lexpr, Expr *pexpr)
{
        Expr *e = new_expr(EXPR_INDEX);
        e->index.lexpr = lexpr;
        e->index.pexpr = pexpr;
        return e;
}


Expr *new_expr_field(Expr *expr, const char *name)
{
        Expr *e = new_expr(EXPR_FIELD);
        e->field.expr = expr;
        e->field.name = name;
        return e;
}


Expr *new_expr_unary(TokenKind op, Expr *expr)
{
        Expr *e = new_expr(EXPR_UNARY);
        e->unary.op = op;
        e->unary.expr = expr;
        return e;
}


Expr *new_expr_binary(TokenKind op, Expr *left, Expr *right)
{
        Expr *e = new_expr(EXPR_BINARY);
        e->binary.op = op;
        e->binary.left = left;
        e->binary.right = right;
        return e;
}


Expr *new_expr_ternary(Expr *cond, Expr *expr, Expr *or_expr)
{
        Expr *e = new_expr(EXPR_TERNARY);
        e->ternary.cond = cond;
        e->ternary.expr = expr;
        e->ternary.or_expr = or_expr;
        return e;
}

#endif

