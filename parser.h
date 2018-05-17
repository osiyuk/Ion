#ifndef ION_PARSER
#define ION_PARSER


Expr *parse_operand(void);
char is_prefix_op();
char is_postfix_op();
char is_binary_op();
char op_precedence(TokenKind);
Expr **parse_expr_list(void);
Expr *parse_postfix(Expr *);
Expr *parse_unary(void);
Expr *parse_binary(char q);
Expr *parse_ternary(void);
Expr *parse_expr(void);

char is_type_modifier();
Typespec **parse_type_list(void);
Typespec *parse_basetype(void);
Typespec *parse_type_modifier(Typespec *base);
Typespec *parse_type(void);


#define unexpected_token "Unexpected token %s in %s"

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
                t = parse_type();
                expect_token(TOKEN_R_PAREN);
                return new_expr_sizeof_type(t);
        }
        if (match_keyword(cast_keyword)) {
                expect_token(TOKEN_L_PAREN);
                t = parse_type();
                expect_token(TOKEN_R_PAREN);
                e = parse_expr();
                return new_expr_cast(t, e);
        }
        
        syntax_error(unexpected_token, token_info(), "expression");
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


Expr *parse_ternary(void)
{
        Expr *e = parse_binary(HIGHEST_PRECEDENCE);
        
        if (match_token(TOKEN_QUESTION)) {
                Expr *then = parse_binary(HIGHEST_PRECEDENCE);
                expect_token(TOKEN_COLON);
                e = new_expr_ternary(e, then, parse_ternary());
        }
        return e;
}


Expr *parse_expr(void)
{
        return parse_ternary();
}


char is_type_modifier()
{
        if (is_token_keyword(const_keyword)
                || is_token(TOKEN_MUL)
                || is_token(TOKEN_L_BRACKET)) {
                        return TRUE;
        }
        return FALSE;
}


Typespec **parse_type_list(void)
{
        Typespec **list = NULL;
        buf_init(list);
        
        if (is_token(TOKEN_R_PAREN)) {
                return list;
        }
        do {
                buf_push(list, parse_type());
        } while (match_token(TOKEN_COMMA));
        
        return list;
}


Typespec *parse_basetype(void)
{
        Typespec *t;
        Typespec **args;
        
        if (match_token(TOKEN_L_PAREN)) {
                t = parse_type();
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
                args = parse_type_list();
                expect_token(TOKEN_R_PAREN);
                t = NULL;
                if (match_token(TOKEN_COLON)) {
                        t = parse_type();
                }
                return new_typespec_function(args, buf_len(args), t);
        }
        
        syntax_error(unexpected_token, token_info(), "type");
        return NULL;
}


Typespec *parse_type_modifier(Typespec *base)
{
        if (match_keyword(const_keyword)) {
                return new_typespec_const(base);
        }
        if (match_token(TOKEN_MUL)) {
                return new_typespec_ptr(base);
        }
        if (match_token(TOKEN_L_BRACKET)) {
                Expr *length = NULL;
                if (!is_token(TOKEN_R_BRACKET)) {
                        length = parse_expr();
                }
                expect_token(TOKEN_R_BRACKET);
                return new_typespec_array(base, length);
        }
        
        syntax_error(unexpected_token, token_info(), "type modifier");
        return NULL;
}


Typespec *parse_type(void)
{
        Typespec *t = parse_basetype();
        
        while (is_type_modifier()) {
                t = parse_type_modifier(t);
        }
        return t;
}

#endif

