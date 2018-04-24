#ifndef ION_PARSER
#define ION_PARSER

#include "ast.h"
#include "lexer.h"

Expr *parse_operand(void);
char is_prefix_op();
char is_postfix_op();
char is_binary_op();
Expr *parse_expr(void);

Typespec *parse_type(void);


#define unexpected_token "Unexpected token %s in %s"

Expr *parse_operand(void)
{
        Expr *e;
        Typespec *t;
        
        switch (token.kind) {
        case TOKEN_L_PAREN:
                e = parse_expr();
                expect_token(TOKEN_R_PAREN);
                return e;
        case TOKEN_NAME:
                e = new_expr_name(token.name);
                next_token();
                return e;
        case TOKEN_INT:
                e = new_expr_int(token.int_val);
                next_token();
                return e;
        case TOKEN_FLOAT:
                e = new_expr_float(token.float_val);
                next_token();
                return e;
        case TOKEN_STR:
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

#endif

