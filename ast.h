#ifndef ABSTRACT_SYNTAX_TREE
#define ABSTRACT_SYNTAX_TREE

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Expr Expr;


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
};

struct Expr {
        enum ExprKind kind;
        union {
                const char *name;
                int64_t int_val;
                double float_val;
                const char *str_val;
        };
};


Expr *expr_new(enum ExprKind kind)
{
        Expr *e = ast_alloc(sizeof(Expr));
        e->kind = kind;
        return e;
}


Expr *expr_name(const char *name)
{
        Expr *e = expr_new(EXPR_NAME);
        e->name = name;
        return e;
}


Expr *expr_int(int64_t int_val)
{
        Expr *e = expr_new(EXPR_INT);
        e->int_val = int_val;
        return e;
}


Expr *expr_float(double float_val)
{
        Expr *e = expr_new(EXPR_FLOAT);
        e->float_val = float_val;
        return e;
}


Expr *expr_str(const char *str_val)
{
        Expr *e = expr_new(EXPR_STR);
        e->str_val = str_val;
        return e;
}

#endif

