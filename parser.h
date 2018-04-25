#ifndef ION_PARSER
#define ION_PARSER

#include "ast.h"
#include "lexer.h"

Expr *parse_operand(void);
char is_prefix_op();
char is_postfix_op();
char is_binary_op();
char op_precedence(TokenKind);
Expr *parse_unary(void);
Expr *parse_expr(void);

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
        }
        return FALSE;
}


char is_postfix_op()
{
        switch (token.kind) {
        case TOKEN_INC:
        case TOKEN_DEC:
        case TOKEN_L_PAREN:
        case TOKEN_L_BRACKET:
        case TOKEN_DOT:
                return TRUE;
        }
        return FALSE;
}


char is_binary_op()
{
        TokenKind k = token.kind;
        
        if (TOKEN_MUL <= k && k <= TOKEN_LOGICAL_OR) {
                return TRUE;
        }
        return FALSE;
}


char op_precedence(TokenKind k)
{
        assert(TOKEN_MUL <= k && k <= TOKEN_LOGICAL_OR);
        
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
        }
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
                Expr **args, *pexpr;
                const char *name;
                
                if (is_token(TOKEN_INC) || is_token(TOKEN_DEC)) {
                        goto postfix;
                }
                if (match_token(TOKEN_L_PAREN)) {
                        goto func_call;
                }
                if (match_token(TOKEN_L_BRACKET)) {
                        goto index_access;
                }
                if (match_token(TOKEN_DOT)) {
                        goto field_access;
                }
                fatal_error("unreachable");
postfix:
                op = token.kind;
                next_token();
                e = new_expr_postfix(op, e);
                continue;
func_call:
                args = NULL;
                buf_init(args);
                
                if (is_token(TOKEN_R_PAREN)) {
                        goto r_paren;
                }
                buf_push(args, parse_expr());
                while (match_token(TOKEN_COMMA)) {
                        buf_push(args, parse_expr());
                }
r_paren:
                expect_token(TOKEN_R_PAREN);
                e = new_expr_call(e, args, buf_len(args));
                continue;
index_access:
                pexpr = parse_expr();
                expect_token(TOKEN_R_BRACKET);
                e = new_expr_index(e, pexpr);
                continue;
field_access:
                name = token.name;
                expect_token(TOKEN_NAME);
                e = new_expr_field(e, name);
                continue;
        }
        return e;
}

#endif

