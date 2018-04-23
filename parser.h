#ifndef ION_PARSER
#define ION_PARSER

#include "ast.h"
#include "lexer.h"

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

#endif

