#ifndef ABSTRACT_SYNTAX_TREE
#define ABSTRACT_SYNTAX_TREE


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
        EXPR_SIZEOF,
        EXPR_SIZEOF_TYPE,
};

struct Expr {
        enum ExprKind kind;
        union {
                const char *name;
                int64_t int_val;
                double float_val;
                const char *str_val;
                Expr *sizeof_expr;
                Typespec *sizeof_type;
                
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
                        Expr *oexpr;
                        Expr *iexpr;
                } index;
                
                struct {
                        Expr *expr;
                        const char *name;
                } field;
                
                struct {
                        TokenKind op;
                        Expr *expr;
                        char is_postfix;
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


Expr *new_expr_index(Expr *oexpr, Expr *iexpr)
{
        Expr *e = new_expr(EXPR_INDEX);
        e->index.oexpr = oexpr;
        e->index.iexpr = iexpr;
        return e;
}


Expr *new_expr_field(Expr *expr, const char *name)
{
        Expr *e = new_expr(EXPR_FIELD);
        e->field.expr = expr;
        e->field.name = name;
        return e;
}


enum {
        PREFIX = 0,
        POSTFIX = 1
};


Expr *new_expr_unary(TokenKind op, Expr *expr)
{
        Expr *e = new_expr(EXPR_UNARY);
        e->unary.op = op;
        e->unary.expr = expr;
        e->unary.is_postfix = PREFIX;
        return e;
}


Expr *new_expr_postfix(TokenKind op, Expr *expr)
{
        Expr *e = new_expr(EXPR_UNARY);
        e->unary.op = op;
        e->unary.expr = expr;
        e->unary.is_postfix = POSTFIX;
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


Expr *new_expr_sizeof(Expr *sizeof_expr)
{
        Expr *e = new_expr(EXPR_SIZEOF);
        e->sizeof_expr = sizeof_expr;
        return e;
}


Expr *new_expr_sizeof_type(Typespec *sizeof_type)
{
        Expr *e = new_expr(EXPR_SIZEOF_TYPE);
        e->sizeof_type = sizeof_type;
        return e;
}


enum TypespecKind {
        TYPESPEC_NONE,
        TYPESPEC_NAME,
        TYPESPEC_CONST,
        TYPESPEC_PTR,
        TYPESPEC_ARRAY,
        TYPESPEC_FUNCTION
};

struct Typespec {
        enum TypespecKind kind;
        union {
                const char *name;
                Typespec *base;
                struct {
                        Typespec *base;
                        Expr *length;
                } array;
                struct {
                        Typespec **args;
                        size_t num_args;
                        Typespec *ret;
                } function;
        };
};


Typespec *new_typespec(enum TypespecKind kind)
{
        Typespec *t = ast_alloc(sizeof(Typespec));
        t->kind = kind;
        return t;
}


Typespec *new_typespec_name(const char *name)
{
        Typespec *t = new_typespec(TYPESPEC_NAME);
        t->name = name;
        return t;
}


Typespec *new_typespec_const(Typespec *base)
{
        Typespec *t = new_typespec(TYPESPEC_CONST);
        t->base = base;
        return t;
}


Typespec *new_typespec_ptr(Typespec *base)
{
        Typespec *t = new_typespec(TYPESPEC_PTR);
        t->base = base;
        return t;
}


Typespec *new_typespec_array(Typespec *base, Expr *length)
{
        Typespec *t = new_typespec(TYPESPEC_CONST);
        t->array.base = base;
        t->array.length = length;
        return t;
}


Typespec *new_typespec_function(
        Typespec **args,
        size_t num_args,
        Typespec *ret
) {
        Typespec *t = new_typespec(TYPESPEC_FUNCTION);
        t->function.args = args;
        t->function.num_args = num_args;
        t->function.ret = ret;
        return t;
}

#endif

