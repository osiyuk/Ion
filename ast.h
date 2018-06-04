#ifndef ABSTRACT_SYNTAX_TREE
#define ABSTRACT_SYNTAX_TREE


typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;
typedef struct Typespec Typespec;

typedef struct StmtList StmtList;
typedef struct SwitchCase SwitchCase;
typedef struct FuncDecl FuncDecl;
typedef struct BoxDecl BoxDecl;


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
                } func;
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
        Typespec *t = new_typespec(TYPESPEC_ARRAY);
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
        t->func.args = args;
        t->func.num_args = num_args;
        t->func.ret = ret;
        return t;
}


enum StmtKind {
        STMT_NONE,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN,
        STMT_IF,
        STMT_WHILE,
        STMT_DO_WHILE,
        STMT_FOR,
        STMT_SWITCH,
        STMT_BLOCK,
        STMT_EXPR,
};

struct Stmt {
        enum StmtKind kind;
        union {
                Expr *expr;
                
                struct {
                        Expr *cond;
                        Stmt *body;
                        Stmt *other;
                } if_stmt;
                
                struct {
                        Expr *cond;
                        Stmt *body;
                } while_stmt;
                
                struct {
                        StmtList *init;
                        Expr *cond;
                        StmtList *step;
                        Stmt *body;
                } for_stmt;
                
                struct {
                        Expr *expr;
                        SwitchCase **cases;
                        size_t num_cases;
                } switch_stmt;
                
                struct {
                        Stmt **stmt;
                        size_t num_stmt;
                } block;
        };
};

struct StmtList {
        Stmt **stmt;
        size_t num_stmt;
};

struct SwitchCase {
        Expr *expr;
        Stmt *stmt;
};


Stmt *new_stmt(enum StmtKind kind)
{
        Stmt *s = ast_alloc(sizeof(Stmt));
        s->kind = kind;
        return s;
}

StmtList *new_stmt_list()
{
        return ast_alloc(sizeof(StmtList));
}

SwitchCase *new_switch_case()
{
        return ast_alloc(sizeof(SwitchCase));
}


#define new_stmt_break() new_stmt(STMT_BREAK)
#define new_stmt_continue() new_stmt(STMT_CONTINUE)


Stmt *new_stmt_return(Expr *expr)
{
        Stmt *s = new_stmt(STMT_RETURN);
        s->expr = expr;
        return s;
}


Stmt *new_stmt_if(Expr *cond, Stmt *body, Stmt *other)
{
        Stmt *s = new_stmt(STMT_IF);
        s->if_stmt.cond = cond;
        s->if_stmt.body = body;
        s->if_stmt.other = other;
        return s;
}


Stmt *new_stmt_while(Expr *cond, Stmt *body)
{
        Stmt *s = new_stmt(STMT_WHILE);
        s->while_stmt.cond = cond;
        s->while_stmt.body = body;
        return s;
}


Stmt *new_stmt_do_while(Stmt *body, Expr *cond)
{
        Stmt *s = new_stmt_while(cond, body);
        s->kind = STMT_DO_WHILE;
        return s;
}


Stmt *new_stmt_for(StmtList *init, Expr *cond, StmtList *step, Stmt *body)
{
        Stmt *s = new_stmt(STMT_FOR);
        s->for_stmt.init = init;
        s->for_stmt.cond = cond;
        s->for_stmt.step = step;
        s->for_stmt.body = body;
        return s;
}


Stmt *new_stmt_switch(Expr *expr, SwitchCase **cases, size_t num_cases)
{
        Stmt *s = new_stmt(STMT_SWITCH);
        s->switch_stmt.expr = expr;
        s->switch_stmt.cases = cases;
        s->switch_stmt.num_cases = num_cases;
        return s;
}


Stmt *new_stmt_block(Stmt **stmt, size_t num_stmt)
{
        Stmt *s = new_stmt(STMT_BLOCK);
        s->block.stmt = stmt;
        s->block.num_stmt = num_stmt;
        return s;
}


Stmt *new_stmt_expr(Expr *expr)
{
        Stmt *s = new_stmt(STMT_EXPR);
        s->expr = expr;
        return s;
}


enum DeclKind {
        DECL_NONE,
        DECL_TYPEDEF,
        DECL_ENUM,
        DECL_STRUCT,
        DECL_UNION,
        DECL_CONST,
        DECL_VAR,
        DECL_FUNC,
};

struct Decl {
        enum DeclKind kind;
        const char *name;
        union {
                Typespec *typespec;
                BoxDecl *box;

                struct {
                        Typespec *type;
                        Expr *expr;
                } var;
                
                struct {
                        FuncDecl *decl;
                        Stmt *body;
                } func;
        };
};

struct BoxDecl {
        const char **names;
        union {
                Typespec **types;
                Expr **exprs;
        };
        size_t num_names;
};

struct FuncDecl {
        const char **args;
        Typespec **types;
        size_t num_args;
        Typespec *ret;
};


Decl *new_decl(enum DeclKind kind, const char *name)
{
        Decl *d = ast_alloc(sizeof(Decl));
        d->kind = kind;
        d->name = name;
        return d;
}

BoxDecl *new_box_decl()
{
        BoxDecl *b = ast_alloc(sizeof(BoxDecl));
        b->names = NULL;
        b->types = NULL;
        return b;
}

FuncDecl *new_func_decl()
{
        FuncDecl *f = ast_alloc(sizeof(FuncDecl));
        f->args = NULL;
        f->types = NULL;
        return f;
}


Decl *new_decl_typedef(const char *name, Typespec *typespec)
{
        Decl *d = new_decl(DECL_TYPEDEF, name);
        d->typespec = typespec;
        return d;
}


Decl *new_decl_box(enum DeclKind kind, const char *name, BoxDecl *box)
{
        Decl *d = new_decl(kind, name);
        d->box = box;
        return d;
}

#define new_decl_enum(name, box) new_decl_box(DECL_ENUM, name, box)
#define new_decl_struct(name, box) new_decl_box(DECL_STRUCT, name, box)
#define new_decl_union(name, box) new_decl_box(DECL_UNION, name, box)


Decl *new_decl_const(const char *name, Typespec *type, Expr *expr)
{
        Decl *d = new_decl(DECL_CONST, name);
        d->var.type = type;
        d->var.expr = expr;
        return d;
}


Decl *new_decl_var(const char *name, Typespec *type, Expr *expr)
{
        Decl *d = new_decl_const(name, type, expr);
        d->kind = DECL_VAR;
        return d;
}


Decl *new_decl_func(const char *name, FuncDecl *decl, Stmt *body)
{
        Decl *d = new_decl(DECL_FUNC, name);
        d->func.decl = decl;
        assert(body->kind == STMT_BLOCK);
        d->func.body = body;
        return d;
}

#endif

