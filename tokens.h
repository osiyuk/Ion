#ifndef ION_TOKENS
#define ION_TOKENS


typedef enum TokenKind TokenKind;

const char *token_kind(TokenKind);


enum TokenKind {
        TOKEN_EOF,
        TOKEN_INC,
        TOKEN_DEC,
        
        TOKEN_L_PAREN,
        TOKEN_R_PAREN,
        TOKEN_L_BRACE,
        TOKEN_R_BRACE,
        TOKEN_L_BRACKET,
        TOKEN_R_BRACKET,
        TOKEN_QUESTION,
        TOKEN_COLON,
        TOKEN_SEMICOLON,
        TOKEN_COMMA,
        TOKEN_DOT,
        TOKEN_NOT,
        TOKEN_NEG,
        
        TOKEN_KEYWORD,
        TOKEN_NAME,
        TOKEN_INT,
        TOKEN_FLOAT,
        TOKEN_STR,
        
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_MOD,
        TOKEN_AND,
        TOKEN_LSHIFT,
        TOKEN_RSHIFT,
        
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_XOR,
        TOKEN_OR,
        
        TOKEN_EQ,
        TOKEN_LT,
        TOKEN_GT,
        TOKEN_LTEQ,
        TOKEN_GTEQ,
        TOKEN_NEQ,
        TOKEN_LOGICAL_AND,
        TOKEN_LOGICAL_OR,
        
        TOKEN_ASSIGN,
        TOKEN_COLON_ASSIGN,
        TOKEN_MUL_ASSIGN,
        TOKEN_DIV_ASSIGN,
        TOKEN_MOD_ASSIGN,
        TOKEN_AND_ASSIGN,
        TOKEN_LSHIFT_ASSIGN,
        TOKEN_RSHIFT_ASSIGN,
        TOKEN_ADD_ASSIGN,
        TOKEN_SUB_ASSIGN,
        TOKEN_XOR_ASSIGN,
        TOKEN_OR_ASSIGN,
};


const char *token_kind_repr[] = {
        [TOKEN_EOF]     = "EOF",
        [TOKEN_INC]     = "++",
        [TOKEN_DEC]     = "--",
        
        [TOKEN_L_PAREN] = "(",
        [TOKEN_R_PAREN] = ")",
        [TOKEN_L_BRACE] = "[",
        [TOKEN_R_BRACE] = "]",
        [TOKEN_L_BRACKET] = "{",
        [TOKEN_R_BRACKET] = "}",
        [TOKEN_QUESTION] = "?",
        [TOKEN_COLON]   = ":",
        [TOKEN_SEMICOLON] = ";",
        [TOKEN_COMMA]   = ",",
        [TOKEN_DOT]     = ".",
        [TOKEN_NOT]     = "!",
        [TOKEN_NEG]     = "~",
        
        [TOKEN_MUL]     = "*",
        [TOKEN_DIV]     = "/",
        [TOKEN_MOD]     = "%",
        [TOKEN_AND]     = "&",
        [TOKEN_LSHIFT]  = "<<",
        [TOKEN_RSHIFT]  = ">>",
        
        [TOKEN_ADD]     = "+",
        [TOKEN_SUB]     = "-",
        [TOKEN_XOR]     = "^",
        [TOKEN_OR]      = "|",
        
        [TOKEN_EQ]      = "==",
        [TOKEN_LT]      = "<",
        [TOKEN_GT]      = ">",
        [TOKEN_LTEQ]    = "<=",
        [TOKEN_GTEQ]    = ">=",
        [TOKEN_NEQ]     = "!=",
        [TOKEN_LOGICAL_AND]     = "&&",
        [TOKEN_LOGICAL_OR]      = "||",
        
        [TOKEN_ASSIGN]  = "=",
        [TOKEN_COLON_ASSIGN]    = ":=",
        [TOKEN_MUL_ASSIGN]      = "*=",
        [TOKEN_DIV_ASSIGN]      = "/=",
        [TOKEN_MOD_ASSIGN]      = "%=",
        [TOKEN_AND_ASSIGN]      = "&=",
        [TOKEN_LSHIFT_ASSIGN]   = "<<=",
        [TOKEN_RSHIFT_ASSIGN]   = ">>=",
        [TOKEN_ADD_ASSIGN]      = "+=",
        [TOKEN_SUB_ASSIGN]      = "-=",
        [TOKEN_XOR_ASSIGN]      = "^=",
        [TOKEN_OR_ASSIGN]       = "|=",
};


const char *token_kind(TokenKind kind)
{
        if (kind > TOKEN_OR_ASSIGN) {
                return "<unknown>";
        }
        assert(token_kind_repr[kind]);
        
        return token_kind_repr[kind];
}

#endif

