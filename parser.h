#ifndef ION_PARSER
#define ION_PARSER

#define unexpected_token "Unexpected token %s in %s"


Expr *parse_operand(void);
char is_prefix_op();
char is_postfix_op();
char is_binary_op();
char op_precedence(TokenKind);
Expr **parse_expr_list(void);
Expr *parse_postfix(Expr *);
Expr *parse_unary(void);
Expr *parse_binary(char q);
Expr *parse_expr(void);

char is_type_modifier();
Typespec **parse_typespec_list(void);
Typespec *parse_basetype(void);
Typespec *parse_typespec_modifier(Typespec *base);
Typespec *parse_typespec(void);

char is_assign_op();
Stmt *parse_statement(void);
SwitchCase *parse_switch_case(void);

Decl *parse_declaration(void);
Decl *parse_enum_decl(const char *name);
Decl *parse_func_decl(const char *name);
Decl *parse_aggregate(const char *name, char is_struct);

Decl **recursive_descent_parser(void);


char match_token(enum TokenKind kind)
{
        if (token.kind == kind) {
                next_token();
                return 1;
        }
        return 0;
}

char match_keyword(char const *keyword)
{
        if (token.kind == TOKEN_KEYWORD && token.name == keyword) {
                next_token();
                return 1;
        }
        return 0;
}

char is_token(enum TokenKind kind)
{
        if (token.kind == kind) {
                return 1;
        }
        return 0;
}

char is_token_keyword(char const *keyword)
{
        if (token.kind == TOKEN_KEYWORD && token.name == keyword) {
                return 1;
        }
        return 0;
}

char expect_token(enum TokenKind kind)
{
        if (match_token(kind)) {
                return 1;
        }
        char const *a, *b;
        a = token_kind(kind);
        b = token_kind(token.kind);
        syntax_error(expected_token, a, b);
        return 0;
}


Expr *parse_operand(void)
{
        Expr *e;
        Typespec *t;
        
        if (match_token(TOKEN_L_PAREN)) {
                e = parse_expr();
                expect_token(TOKEN_R_PAREN);
                return e;
        }
        if (is_token(TOKEN_NAME)) {
                e = new_expr_name(token.name);
                next_token();
                return e;
        }
        if (is_token(TOKEN_INT)) {
                e = new_expr_int(token.int_val);
                next_token();
                return e;
        }
        if (is_token(TOKEN_FLOAT)) {
                e = new_expr_float(token.float_val);
                next_token();
                return e;
        }
        if (is_token(TOKEN_STR)) {
                e = new_expr_str(token.str_val);
                next_token();
                return e;
        }
        if (match_keyword(sizeof_keyword)) {
                expect_token(TOKEN_L_PAREN);
                if (match_token(TOKEN_COLON)) {
                        goto sizeof_type;
                }
                e = parse_expr();
                expect_token(TOKEN_R_PAREN);
                return new_expr_sizeof(e);
sizeof_type:
                t = parse_typespec();
                expect_token(TOKEN_R_PAREN);
                return new_expr_sizeof_type(t);
        }
        if (match_keyword(cast_keyword)) {
                expect_token(TOKEN_L_PAREN);
                t = parse_typespec();
                expect_token(TOKEN_R_PAREN);
                e = parse_expr();
                return new_expr_cast(t, e);
        }
        
        if (is_token(TOKEN_EOF)) {
                fatal_error("Unexpected end of file");
        }
        fatal_error(unexpected_token, token_kind(token.kind), "operand");
        return NULL;
}


enum {
        FALSE = 0,
        TRUE = 1
};


char is_prefix_op()
{
        switch (token.kind) {
        case TOKEN_INC:
        case TOKEN_DEC:
        case TOKEN_NOT:
        case TOKEN_NEG:
        case TOKEN_MUL:
        case TOKEN_AND:
        case TOKEN_ADD:
        case TOKEN_SUB:
                return TRUE;
        default:
                return FALSE;
        }
}


char is_postfix_op()
{
        switch (token.kind) {
        case TOKEN_INC:
        case TOKEN_DEC:
        case TOKEN_L_PAREN:
        case TOKEN_L_BRACE:
        case TOKEN_DOT:
                return TRUE;
        default:
                return FALSE;
        }
}


char is_binary_op()
{
        TokenKind k = token.kind;
        
        if (TOKEN_MUL <= k && k <= TOKEN_LOGICAL_OR) {
                return TRUE;
        }
        return FALSE;
}


const int HIGHEST_PRECEDENCE = 0;


char op_precedence(TokenKind k)
{
        assert(TOKEN_MUL <= k);
        
        switch (k) {
        /* addition-multiplication group */
        case TOKEN_MUL:
        case TOKEN_DIV:
        case TOKEN_MOD:
                return 1;
        case TOKEN_ADD:
        case TOKEN_SUB:
                return 2;
        /* bitwise operator group */
        case TOKEN_LSHIFT:
        case TOKEN_RSHIFT:
                return 3;
        case TOKEN_AND:
                return 4;
        case TOKEN_XOR:
                return 5;
        case TOKEN_OR:
                return 6;
        /* relational operator group */
        case TOKEN_LT:
        case TOKEN_LTEQ:
        case TOKEN_GT:
        case TOKEN_GTEQ:
                return 7;
        case TOKEN_EQ:
        case TOKEN_NEQ:
                return 8;
        /* logical operator group */
        case TOKEN_LOGICAL_AND:
                return 9;
        case TOKEN_LOGICAL_OR:
                return 10;
        default:
                assert(k <= TOKEN_LOGICAL_OR);
                return 0;
        }
}


Expr **parse_expr_list(void)
{
        Expr **list = NULL;
        buf_init(list);
        
        if (is_token(TOKEN_R_PAREN)) {
                return list;
        }
        do {
                buf_push(list, parse_expr());
        } while (match_token(TOKEN_COMMA));
        
        return list;
}


Expr *parse_postfix(Expr *expr)
{
        Expr **args;
        
        if (is_token(TOKEN_INC) || is_token(TOKEN_DEC)) {
                TokenKind op = token.kind;
                next_token();
                return new_expr_postfix(op, expr);
        }
        if (match_token(TOKEN_DOT)) {
                const char *name = token.name;
                expect_token(TOKEN_NAME);
                return new_expr_field(expr, name);
        }
        if (match_token(TOKEN_L_BRACE)) {
                Expr *index = parse_expr();
                expect_token(TOKEN_R_BRACE);
                return new_expr_index(expr, index);
        }
        expect_token(TOKEN_L_PAREN);
        args = parse_expr_list();
        expect_token(TOKEN_R_PAREN);
        return new_expr_call(expr, args, buf_len(args));
}


Expr *parse_unary(void)
{
        TokenKind op;
        Expr *e;
        
        if (is_prefix_op()) {
                op = token.kind;
                next_token();
                e = parse_unary();
                return new_expr_unary(op, e);
        }
        e = parse_operand();
        
        while (is_postfix_op()) {
                e = parse_postfix(e);
        }
        return e;
}


Expr *parse_binary(char q)
{
        TokenKind op;
        Expr *e, *r;
        char p;
        
        e = parse_unary();
        
        while (is_binary_op()) {
                op = token.kind;
                next_token();
                p = op_precedence(op);
                
                if (q <= p) {
                        q = p;
                        e = new_expr_binary(op, e, parse_unary());
                        continue;
                }
                r = parse_binary(HIGHEST_PRECEDENCE);
                e->binary.right = new_expr_binary(op, e->binary.right, r);
        }
        return e;
}


Expr *parse_expr(void)
{
        Expr *e = parse_binary(HIGHEST_PRECEDENCE);

        if (is_assign_op()) {
                TokenKind op = token.kind;
                next_token();
                e = new_expr_binary(op, e, parse_expr());
        }
        if (match_token(TOKEN_QUESTION)) {
                Expr *then = parse_binary(HIGHEST_PRECEDENCE);
                expect_token(TOKEN_COLON);
                e = new_expr_ternary(e, then, parse_expr());
        }
        return e;
}


char is_type_modifier()
{
        if (is_token_keyword(const_keyword) ||
                is_token(TOKEN_MUL) ||
                is_token(TOKEN_L_BRACE)) {
                        return TRUE;
        }
        return FALSE;
}


Typespec **parse_typespec_list(void)
{
        Typespec **list = NULL;
        buf_init(list);
        
        if (is_token(TOKEN_R_PAREN)) {
                return list;
        }
        do {
                buf_push(list, parse_typespec());
        } while (match_token(TOKEN_COMMA));
        
        return list;
}


Typespec *parse_basetype(void)
{
        Typespec *t;
        Typespec **args;
        
        if (match_token(TOKEN_L_PAREN)) {
                t = parse_typespec();
                expect_token(TOKEN_R_PAREN);
                return t;
        }
        if (is_token(TOKEN_NAME)) {
                t = new_typespec_name(token.name);
                next_token();
                return t;
        }
        if (match_keyword(func_keyword)) {
                expect_token(TOKEN_L_PAREN);
                args = parse_typespec_list();
                expect_token(TOKEN_R_PAREN);
                t = NULL;
                if (match_token(TOKEN_COLON)) {
                        t = parse_typespec();
                }
                return new_typespec_function(args, buf_len(args), t);
        }
        
        fatal_error(unexpected_token, token_kind(token.kind), "type");
        return NULL;
}


Typespec *parse_typespec_modifier(Typespec *base)
{
        if (match_keyword(const_keyword)) {
                return new_typespec_const(base);
        }
        if (match_token(TOKEN_MUL)) {
                return new_typespec_ptr(base);
        }
        if (match_token(TOKEN_L_BRACE)) {
                Expr *length = NULL;
                if (!is_token(TOKEN_R_BRACE)) {
                        length = parse_expr();
                }
                expect_token(TOKEN_R_BRACE);
                return new_typespec_array(base, length);
        }
        
        syntax_error(unexpected_token, token_kind(token.kind), "type modifier");
        return NULL;
}


Typespec *parse_typespec(void)
{
        Typespec *t = parse_basetype();
        
        while (is_type_modifier()) {
                t = parse_typespec_modifier(t);
        }
        return t;
}


char is_assign_op()
{
        TokenKind k = token.kind;
        
        if (TOKEN_ASSIGN <= k && k <= TOKEN_OR_ASSIGN) {
                return TRUE;
        }
        return FALSE;
}


Stmt *parse_statement(void)
{
        Expr *e;
        Stmt *s;
        
        if (match_keyword(break_keyword)) {
                return new_stmt_break();
        }
        if (match_keyword(continue_keyword)) {
                return new_stmt_continue();
        }
        if (match_keyword(return_keyword)) {
                if (is_token(TOKEN_SEMICOLON)) {
                        return new_stmt_return(NULL);
                }
                e = parse_expr();
                return new_stmt_return(e);
        }
        if (match_keyword(if_keyword)) {
                expect_token(TOKEN_L_PAREN);
                e = parse_expr();
                expect_token(TOKEN_R_PAREN);
                s = parse_statement();
                if (!is_token_keyword(else_keyword)) {
                        return new_stmt_if(e, s, NULL);
                }
                match_keyword(else_keyword);
                return new_stmt_if(e, s, parse_statement());
        }
        if (match_keyword(while_keyword)) {
                expect_token(TOKEN_L_PAREN);
                e = parse_expr();
                expect_token(TOKEN_R_PAREN);
                s = parse_statement();
                return new_stmt_while(e, s);
        }
        if (match_keyword(do_keyword)) {
                s = parse_statement();
                if (!is_token_keyword(while_keyword)) {
                        syntax_error("Expected while keyword");
                }
                match_keyword(while_keyword);
                expect_token(TOKEN_L_PAREN);
                e = parse_expr();
                expect_token(TOKEN_R_PAREN);
                return new_stmt_do_while(s, e);
        }
        if (match_keyword(for_keyword)) {
                Expr *init, *cond, *step;
                expect_token(TOKEN_L_PAREN);
                init = NULL;
                if (!is_token(TOKEN_SEMICOLON)) {
                        init = parse_expr();
                }
                expect_token(TOKEN_SEMICOLON);
                cond = NULL;
                if (!is_token(TOKEN_SEMICOLON)) {
                        cond = parse_expr();
                }
                expect_token(TOKEN_SEMICOLON);
                step = NULL;
                if (!is_token(TOKEN_R_PAREN)) {
                        step = parse_expr();
                }
                expect_token(TOKEN_R_PAREN);
                s = parse_statement();
                return new_stmt_for(init, cond, step, s);
        }
        if (match_keyword(switch_keyword)) {
                SwitchCase **cases = NULL;
                expect_token(TOKEN_L_PAREN);
                e = parse_expr();
                expect_token(TOKEN_R_PAREN);
                expect_token(TOKEN_L_BRACKET);
                while (!is_token(TOKEN_R_BRACKET)) {
                        buf_push(cases, parse_switch_case());
                }
                expect_token(TOKEN_R_BRACKET);
                if (cases == NULL) {
                        return new_stmt_switch(e, NULL, 0);
                }
                return new_stmt_switch(e, cases, buf_len(cases));
        }
        if (match_token(TOKEN_L_BRACKET)) {
                Stmt **statements = NULL;
                while (!is_token(TOKEN_R_BRACKET)) {
                        buf_push(statements, parse_statement());
                        match_token(TOKEN_SEMICOLON);
                }
                expect_token(TOKEN_R_BRACKET);
                if (statements == NULL) {
                        return new_stmt_block(NULL, 0);
                }
                return new_stmt_block(statements, buf_len(statements));
        }
        if (match_token(TOKEN_SEMICOLON)) {
                return new_stmt_block(NULL, 0);
        }
        
        e = parse_expr();
        return new_stmt_expr(e);
}


SwitchCase *parse_switch_case(void)
{
        SwitchCase *sc = new_switch_case();
        Stmt **stmts = NULL;
        
        sc->expr = NULL;
        if (    !is_token_keyword(case_keyword) &&
                !is_token_keyword(default_keyword)) {
                        syntax_error("Expected case or default keywords");
        }
        if (match_keyword(case_keyword)) {
                sc->expr = parse_expr();
        } else {
                match_keyword(default_keyword);
        }
        expect_token(TOKEN_COLON);
        
        while ( !is_token_keyword(case_keyword) &&
                !is_token_keyword(default_keyword) &&
                !is_token(TOKEN_R_BRACKET)) {
                        buf_push(stmts, parse_statement());
                        match_token(TOKEN_SEMICOLON);
        }
        if (stmts == NULL) {
                sc->stmt = NULL;
                return sc;
        }
        if (buf_len(stmts) == 1) {
                sc->stmt = stmts[0];
        } else {
                sc->stmt = new_stmt_block(stmts, buf_len(stmts));
        }
        return sc;
}


Decl *parse_declaration(void)
{
        const char *name;
        Typespec *type;
        
        if (match_keyword(typedef_keyword)) {
                name = token.name;
                expect_token(TOKEN_NAME);
                expect_token(TOKEN_ASSIGN);
                type = parse_typespec();
                return new_decl_typedef(name, type);
        }
        if (match_keyword(enum_keyword)) {
                name = token.name;
                expect_token(TOKEN_NAME);
                expect_token(TOKEN_L_BRACKET);
                return parse_enum_decl(name);
        }
        if (    is_token_keyword(struct_keyword) ||
                is_token_keyword(union_keyword))
        {
                char is_struct = is_token_keyword(struct_keyword);
                next_token();
                name = token.name;
                expect_token(TOKEN_NAME);
                return parse_aggregate(name, is_struct);
        }
        if (match_keyword(const_keyword)) {
                Expr *expr;
                
                name = token.name;
                expect_token(TOKEN_NAME);
                expect_token(TOKEN_ASSIGN);
                expr = parse_expr();
                return new_decl_const(name, NULL, expr);
        }
        if (match_keyword(var_keyword)) {
                Expr *expr;
                
                name = token.name;
                expect_token(TOKEN_NAME);
                if (match_token(TOKEN_ASSIGN)) {
                        expr = parse_expr();
                        return new_decl_var(name, NULL, expr);
                }
                expect_token(TOKEN_COLON);
                type = parse_typespec();
                if (!match_token(TOKEN_ASSIGN)) {
                        return new_decl_var(name, type, NULL);
                }
                expr = parse_expr();
                return new_decl_var(name, type, expr);
        }
        if (match_keyword(func_keyword)) {
                name = token.name;
                expect_token(TOKEN_NAME);
                expect_token(TOKEN_L_PAREN);
                return parse_func_decl(name);
        }
        syntax_error("Expected declaration got %s", token_kind(token.kind));
        return NULL;
}


Decl *parse_enum_decl(const char *name)
{
        BoxDecl *box = new_box_decl();
        
        while (!is_token(TOKEN_R_BRACKET)) {
                if (!is_token(TOKEN_NAME)) {
                        syntax_error("Expect enum constant name");
                        return NULL;
                }
                buf_push(box->names, token.name);
                match_token(TOKEN_NAME);
                if (match_token(TOKEN_ASSIGN)) {
                        buf_push(box->exprs, parse_expr());
                } else {
                        buf_push(box->exprs, NULL);
                }
                match_token(TOKEN_COMMA);
        }
        expect_token(TOKEN_R_BRACKET);
        if (box->names) {
                box->num_names = buf_len(box->names);
                assert(box->num_names == buf_len(box->exprs));
        }
        return new_decl_enum(name, box);
}


Decl *parse_func_decl(const char *name)
{
        FuncDecl *decl = NULL;
        Stmt **stmt = NULL;
        Stmt *body;
        
        decl = new_func_decl();
        
        while (!is_token(TOKEN_R_PAREN)) {
                if (!is_token(TOKEN_NAME)) {
                        syntax_error("Expect function param name");
                }
                buf_push(decl->args, token.name);
                expect_token(TOKEN_NAME);
                if (is_token(TOKEN_COMMA)) {
                        syntax_error("multiple args of single type should be declared separately");
                }
                expect_token(TOKEN_COLON);
                buf_push(decl->types, parse_typespec());
                if (!match_token(TOKEN_COMMA))
                        break;
        }
        expect_token(TOKEN_R_PAREN);
        
        decl->num_args = buf__len(decl->args);
        assert(buf__len(decl->types) == decl->num_args);
        
        decl->ret = NULL;
        if (match_token(TOKEN_COLON)) {
                decl->ret = parse_typespec();
        }
        expect_token(TOKEN_L_BRACKET);
        while (!is_token(TOKEN_R_BRACKET)) {
                buf_push(stmt, parse_statement());
                match_token(TOKEN_SEMICOLON);
        }
        expect_token(TOKEN_R_BRACKET);
        
        body = new_stmt_block(stmt, buf__len(stmt));
        return new_decl_func(name, decl, body);
}


Decl *parse_aggregate(const char *name, char is_struct)
{
        BoxDecl *box = new_box_decl();
        
        expect_token(TOKEN_L_BRACKET);
        while (!is_token(TOKEN_R_BRACKET)) {
                if (!is_token(TOKEN_NAME)) {
                        syntax_error("Expect aggregate field name");
                        return NULL;
                }
                buf_push(box->names, token.name);
                expect_token(TOKEN_NAME);
                if (is_token(TOKEN_COMMA)) {
                        syntax_error("multiple fields of single type should be declared separately");
                        return NULL;
                }
                expect_token(TOKEN_COLON);
                buf_push(box->types, parse_typespec());
                match_token(TOKEN_SEMICOLON);
        }
        expect_token(TOKEN_R_BRACKET);
        if (box->names) {
                box->num_names = buf_len(box->names);
                assert(box->num_names == buf_len(box->types));
        }
        if (is_struct)
                return new_decl_struct(name, box);
        else    return new_decl_union(name, box);
}


Decl **recursive_descent_parser(void)
{
        Decl *decl, **ast = NULL;

        while (!is_token(TOKEN_EOF)) {
                decl = parse_declaration();
                if (decl == NULL) break;
                buf_push(ast, decl);
        }
        return ast;
}

#endif

