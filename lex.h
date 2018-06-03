#ifndef ION_LEXING
#define ION_LEXING


void next_token();
const char *token_info();
char match_keyword(const char *keyword);
char is_token_keyword(const char *name);
char is_token(TokenKind);
char match_token(TokenKind);
char expect_token(TokenKind);


struct Token token;
const char *stream;

#define init_stream(s) stream = s; next_token()


void init_lex(const char *name, const char *content)
{
        filename = name;
        line_number = 1;
        stream = content;
        next_token();
}


uint8_t char_to_digit[] = {
        ['0'] = 0,
        ['1'] = 1,
        ['2'] = 2,
        ['3'] = 3,
        ['4'] = 4,
        ['5'] = 5,
        ['6'] = 6,
        ['7'] = 7,
        ['8'] = 8,
        ['9'] = 9,
        ['a'] = 10, ['A'] = 10,
        ['b'] = 11, ['B'] = 11,
        ['c'] = 12, ['C'] = 12,
        ['d'] = 13, ['D'] = 13,
        ['e'] = 14, ['E'] = 14,
        ['f'] = 15, ['F'] = 15,
};

uint8_t escaped_char[] = {
        ['0'] = 0,
        ['"'] = '"',
        ['\''] = '\'',
        ['\\'] = '\\',
        ['n'] = '\n',
        ['r'] = '\r',
        ['t'] = '\t',
        ['v'] = '\v',
};


#define invalid_digit "invalid digit \"%d\" in octal constant"
#define unknown_escape "unknown escape sequence '\\%c'"
#define missing_term "missing terminating %c character"
#define unknown_token "unknown token '%c' %d, skipping"
#define expected_token "Expected token %s, got %s"


void scan_int(char base)
{
        char digit;
        uint64_t val = 0;
        
        while (isxdigit(*stream)) {
                digit = char_to_digit[(int) *stream];
                
                if (digit >= base) {
                        syntax_error(invalid_digit, digit);
                        digit = 0;
                }
                
                val = val * base + digit;
                stream++;
        }
        token.int_val = val;
}


void scan_float()
{
        double val;
        const char *s = stream;
        
        while (isdigit(*s)) s++;
        if (*s == '.') s++;
        while (isdigit(*s)) s++;
        
        if (tolower(*s) == 'e') {
                s++;
                if (*s == '+' || *s == '-') s++;
                
                if (!isdigit(*s)) {
                        syntax_error("exponent has no digits");
                }
                while (isdigit(*s)) s++;
        }
        
        val = strtod(stream, NULL);
        token.float_val = val;
        stream = s;
}


void scan_str()
{
        char *str = NULL;
        char c;
        
        assert(*stream == '"');
        stream++;
        
        while (*stream && *stream != '"') {
                c = *stream;
                switch (c) {
                case '\\':
                        stream++;
                        c = escaped_char[(int) *stream];
                        if (c == 0 && *stream != '0') {
                                syntax_error(unknown_escape, *stream);
                        }
                        break;
                case '\n':
                        goto missing;
                }
                buf_push(str, c);
                stream++;
        }
        if (!*stream) {
missing:
                syntax_error(missing_term, '"');
                token.str_val = NULL;
                return;
        }
        
        assert(*stream == '"');
        stream++;
        
        buf_push(str, 0);
        token.str_val = str;
}


void scan_char()
{
        char c = 0;
        
        assert(*stream == '\'');
        stream++;
        
        switch (*stream) {
        case '\\':
                stream++;
                c = escaped_char[(int) *stream];
                if (c == 0 && *stream != '0') {
                        syntax_error(unknown_escape, *stream);
                }
                stream++;
                break;
        case '\'':
        case '\n':
                syntax_error(missing_term, '\'');
                stream++;
                token.int_val = 0;
                return;
        default:
                c = *stream;
                stream++;
        }
        
        assert(*stream == '\'');
        stream++;
        
        token.int_val = c;
}


void next_token()
{
        char c, base;
        const char *str;
repeat:
        switch (*stream) {
        case 0:
                token.kind = TOKEN_EOF;
                return;
        case '\n':
                line_number++;
        case ' ':
                stream++;
                goto repeat;
        case '1'...'9':
                base = 10;
                str = stream;
                while (isdigit(*str)) {
                        str++;
                }
                c = *str;
                if (c == '.' || tolower(c) == 'e') {
                        goto _float;
                }
_int:
                token.kind = TOKEN_INT;
                scan_int(base);
                return;
_float:
                token.kind = TOKEN_FLOAT;
                scan_float();
                return;
        case '0':
                base = 8;
                stream++;
                if (*stream == 'x') {
                        base = 16;
                        stream++;
                }
                goto _int;
        case '.':
                if (isdigit(stream[1])) {
                        goto _float;
                }
                token.kind = TOKEN_DOT;
                stream++;
                return;
        case 34: // "
                token.kind = TOKEN_STR;
                scan_str();
                return;
        case 39: // '
                token.kind = TOKEN_INT;
                scan_char();
                return;
#define CASE(c, k) \
        case c: \
                token.kind = k; \
                stream++; \
                return;
        CASE('(', TOKEN_L_PAREN)
        CASE(')', TOKEN_R_PAREN)
        CASE('[', TOKEN_L_BRACE)
        CASE(']', TOKEN_R_BRACE)
        CASE('{', TOKEN_L_BRACKET)
        CASE('}', TOKEN_R_BRACKET)
        CASE('?', TOKEN_QUESTION)
#define CASE2(c, k, c1, k1) \
        case c: \
                token.kind = k; \
                stream++; \
                if (*stream == c1) { \
                        token.kind = k1; \
                        stream++;\
                } \
                return;
        CASE2('=', TOKEN_ASSIGN, '=', TOKEN_EQ)
        CASE2(':', TOKEN_COLON, '=', TOKEN_COLON_ASSIGN)
        CASE(';', TOKEN_SEMICOLON)
        CASE(',', TOKEN_COMMA)
        CASE2('!', TOKEN_NOT, '=', TOKEN_NEQ)
        CASE('~', TOKEN_NEG)
        CASE2('*', TOKEN_MUL, '=', TOKEN_MUL_ASSIGN)
        CASE2('/', TOKEN_DIV, '=', TOKEN_DIV_ASSIGN)
        CASE2('%', TOKEN_MOD, '=', TOKEN_MOD_ASSIGN)
#define CASE3(c, k, c1, k1, c2, k2) \
        case c: \
                token.kind = k; \
                stream++; \
                if (*stream == c1) { \
                        token.kind = k1; \
                        stream++; \
                } else if (*stream == c2) { \
                        token.kind = k2; \
                        stream++; \
                } \
                return;
        CASE3('&', TOKEN_AND, '=', TOKEN_AND_ASSIGN, '&', TOKEN_LOGICAL_AND)
        CASE3('+', TOKEN_ADD, '=', TOKEN_ADD_ASSIGN, '+', TOKEN_INC)
        CASE3('-', TOKEN_SUB, '=', TOKEN_SUB_ASSIGN, '-', TOKEN_DEC)
        CASE2('^', TOKEN_XOR, '=', TOKEN_XOR_ASSIGN)
        CASE3('|', TOKEN_OR, '=', TOKEN_OR_ASSIGN, '|', TOKEN_LOGICAL_OR)
#define CASE4(c, k, c1, k1, k2, k3) \
        case c: \
                token.kind = k; \
                stream++; \
                if (*stream == c1) { \
                        token.kind = k1; \
                        stream++; \
                } else if (*stream == c) { \
                        token.kind = k2; \
                        stream++; \
                        if (*stream == c1) { \
                                token.kind = k3; \
                                stream++; \
                        } \
                } \
                return;
        CASE4('<', TOKEN_LT, '=', TOKEN_LTEQ, TOKEN_LSHIFT, TOKEN_LSHIFT_ASSIGN)
        CASE4('>', TOKEN_GT, '=', TOKEN_GTEQ, TOKEN_RSHIFT, TOKEN_RSHIFT_ASSIGN)
        case 'a'...'z':
        case 'A'...'Z':
        case '_':
                str = stream++;
                while (isalnum(*stream) || *stream == '_') {
                        stream++;
                }
                
                token.name = str_intern_slice(str, stream - str);
                token.kind = is_keyword(token.name) ? TOKEN_KEYWORD : TOKEN_NAME;
                break;
        default:
                c = *stream;
                syntax_error(unknown_token, c, c);
                stream++;
        }
}


const char *token_info()
{
        if (is_token(TOKEN_KEYWORD) || is_token(TOKEN_NAME)) {
                return token.name;
        }
        return token_kind(token.kind);
}


char match_keyword(const char *name)
{
        if (is_token_keyword(name)) {
                next_token();
                return 1;
        }
        return 0;
}


char is_token_keyword(const char *name)
{
        if (is_token(TOKEN_KEYWORD) && token.name == name) {
                return 1;
        }
        return 0;
}


char is_token(TokenKind kind)
{
        if (token.kind == kind) {
                return 1;
        }
        return 0;
}


char match_token(TokenKind kind)
{
        if (is_token(kind)) {
                next_token();
                return 1;
        }
        return 0;
}


char expect_token(TokenKind kind)
{
        if (match_token(kind)) {
                return 1;
        }
        syntax_error(expected_token, token_kind(kind), token_info());
        return 0;
}

#endif

