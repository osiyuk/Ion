#ifndef AST_PRINT_AUTOMATON
#define AST_PRINT_AUTOMATON


char use_print_buf, *print_buf;

#define buf_printf(...) (print_buf = buf_printf(print_buf, __VA_ARGS__))
#define printf(...) \
        use_print_buf ? (void) buf_printf(__VA_ARGS__) : printf(__VA_ARGS__)


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
                printf("(call ");
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


void print_type(Typespec *type)
{
        const Typespec t = *type;
        
        switch (type->kind) {
        case TYPESPEC_NAME:
                printf(type->name);
                return;
        case TYPESPEC_CONST:
                printf("(const ");
                print_type(type->base);
                printf(")");
                return;
        case TYPESPEC_PTR:
                printf("(ptr ");
                print_type(type->base);
                printf(")");
                return;
        case TYPESPEC_ARRAY:
                printf("(array ");
                print_type(t.array.base);
                printf(" ");
                print_expr(t.array.length);
                printf(")");
                return;
        case TYPESPEC_FUNCTION:
                printf("(func (");
                for (size_t i = 0; i < t.func.num_args; i++) {
                        if (i) printf(", ");
                        print_type(t.func.args[i]);
                }
                printf(") ");
                if (t.func.ret) print_type(t.func.ret);
                else printf("void");
                printf(")");
                return;
        default:
                assert(TYPESPEC_NONE);
        }
}


void print_stmt(Stmt *stmt)
{
        const Stmt s = *stmt;
        
        switch (stmt->kind) {
        case STMT_BREAK:
                printf("(break)");
                return;
        case STMT_CONTINUE:
                printf("(continue)");
                return;
        case STMT_RETURN:
                printf("(return");
                if (stmt->expr) {
                        printf(" ");
                        print_expr(stmt->expr);
                }
                printf(")");
                return;
        case STMT_IF:
                printf("(if ");
                print_expr(s.if_stmt.cond);
                printf(" ");
                print_stmt(s.if_stmt.body);
                if (s.if_stmt.other) {
                        printf(" else ");
                        print_stmt(s.if_stmt.other);
                }
                printf(")");
                return;
        case STMT_WHILE:
                printf("(while ");
                print_expr(s.while_stmt.cond);
                printf(" ");
                print_stmt(s.while_stmt.body);
                printf(")");
                return;
        case STMT_DO_WHILE:
                printf("(do ");
                print_stmt(s.while_stmt.body);
                printf(" while ");
                print_expr(s.while_stmt.cond);
                printf(")");
                return;
        case STMT_FOR:
                printf("(for ");
                if (s.for_stmt.init) {
                        StmtList *init = s.for_stmt.init;
                        for (size_t i = 0; i < init->num_stmt; i++) {
                                if (i) printf(", ");
                                print_stmt(init->stmt[i]);
                        }
                } else {
                        printf("()");
                }
                printf(" ");
                if (s.for_stmt.cond) {
                        print_expr(s.for_stmt.cond);
                } else {
                        printf("()");
                }
                printf(" ");
                if (s.for_stmt.step) {
                        StmtList *step = s.for_stmt.step;
                        for (size_t i = 0; i < step->num_stmt; i++) {
                                if (i) printf(", ");
                                print_stmt(step->stmt[i]);
                        }
                } else {
                        printf("()");
                }
                printf(" ");
                print_stmt(s.for_stmt.body);
                printf(")");
                return;
        case STMT_SWITCH:
                printf("(switch ");
                print_expr(s.switch_stmt.expr);
                for (size_t i = 0; i < s.switch_stmt.num_cases; i++) {
                        SwitchCase *sc = s.switch_stmt.cases[i];
                        printf(" ");
                        if (sc->expr) {
                                printf("(case ");
                                print_expr(sc->expr);
                                printf(" ");
                                print_stmt(sc->stmt);
                                printf(")");
                        } else {
                                printf("(default ");
                                print_stmt(sc->stmt);
                                printf(")");
                        }
                }
                printf(")");
                return;
        case STMT_BLOCK:
                printf("(block");
                if (s.block.num_stmt == 0) {
                        printf(" nil)");
                        return;
                }
                for (size_t i = 0; i < s.block.num_stmt; i++) {
                        printf(" ");
                        print_stmt(s.block.stmt[i]);
                }
                printf(")");
                return;
        case STMT_EXPR:
                print_expr(stmt->expr);
                return;
        case STMT_ASSIGN:
                printf("(%s ", token_kind(s.assign.op));
                print_expr(s.assign.lvalue);
                printf(" ");
                print_stmt(s.assign.rvalue);
                printf(")");
                return;
        default:
                assert(STMT_NONE);
        }
}


void print_decl(Decl *decl)
{
        const Decl d = *decl;
        const FuncDecl *f;
        
        switch (decl->kind) {
        case DECL_TYPEDEF:
                printf("(typedef %s ", decl->name);
                print_type(d.typedef_decl.type);
                printf(")");
                return;
        case DECL_ENUM:
                printf("(enum %s ", decl->name);
                for (size_t i = 0; i < d.enum_decl.num_names; i++) {
                        const char *name = d.enum_decl.names[i];
                        Expr *init_expr = d.enum_decl.init_exprs[i];
                        
                        if (i) printf(" ");
                        if (init_expr) {
                                printf("(%s ", name);
                                print_expr(init_expr);
                                printf(")");
                        } else {
                                printf(name);
                        }
                }
                printf(")");
                return;
        case DECL_STRUCT:
                printf("(struct %s ", decl->name);
                goto aggregate;
        case DECL_UNION:
                printf("(union %s ", decl->name);
aggregate:
                for (size_t i = 0; i < d.aggregate.num_names; i++) {
                        const char *name = d.aggregate.names[i];
                        Typespec *type = d.aggregate.types[i];
                        
                        if (i) printf(" ");
                        printf("(%s ", name);
                        print_type(type);
                        printf(")");
                }
                printf(")");
                return;
        case DECL_CONST:
                printf("(const %s ", decl->name);
                print_expr(d.var.expr);
                printf(")");
                return;
        case DECL_VAR:
                printf("(var %s (", decl->name);
                if (d.var.type) print_type(d.var.type);
                printf(")");
                if (d.var.expr) {
                        printf(" ");
                        print_expr(d.var.expr);
                }
                printf(")");
                return;
        case DECL_FUNC:
                printf("(func %s", decl->name);
                f = d.func.decl;
                for (size_t i = 0; i < f->num_args; i++) {
                        printf(" (%s ", f->args[i]);
                        print_type(f->types[i]);
                        printf(")");
                }
                printf(" (ret ");
                if (f->ret) print_type(f->ret);
                else printf("void");
                printf(") ");
                print_stmt(d.func.body);
                printf(")");
                return;
        default:
                assert(DECL_NONE);
        }
}


void print_ast(Decl **ast, size_t len)
{
        for (size_t i = 0; i < len; i++) {
                print_decl(ast[i]);
                printf("\n");
        }
}

#undef printf
#endif

