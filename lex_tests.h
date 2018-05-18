#ifndef LEXER_REGRESSION_TESTS
#define LEXER_REGRESSION_TESTS

#define assert_token(kind) assert(match_token(kind))
#define assert_token_int(x) assert(token.int_val == (x) && match_token(TOKEN_INT))
#define assert_token_float(x) assert(token.float_val == (x) && match_token(TOKEN_FLOAT))
#define assert_token_empty_str() assert(strlen(token.str_val) == 0 && match_token(TOKEN_STR))
#define assert_token_str(x) assert(strcmp(token.str_val, (x)) == 0 && match_token(TOKEN_STR))
#define assert_token_name(x) assert(strcmp(token.name, (x)) == 0 && match_token(TOKEN_NAME))
#define assert_token_keyword(x) assert(token.name == (x) && match_token(TOKEN_KEYWORD))
#define assert_token_eof() assert(token.kind == TOKEN_EOF)


void lex_integer_literal_tests()
{
        init_stream("0 1 2147990990 18446744073709551615");
        assert_token_int(0);
        assert_token_int(1);
        assert_token_int(2147990990);
        assert_token_int(18446744073709551615u);
        assert_token_eof();
        
        init_stream("013 0xFF 0x00abcdef 0xffffffffffffffff");
        assert_token_int(013);
        assert_token_int(0xFF);
        assert_token_int(0x00abcdef);
        assert_token_int(0xffffffffffffffff);
        assert_token_eof();
}


void lex_float_literal_tests()
{
        init_stream("3.1415 .318 1e0 1. 13E200");
        assert_token_float(3.1415);
        assert_token_float(.318);
        assert_token_float(1e0);
        assert_token_float(1.);
        assert_token_float(13E200);
        assert_token_eof();
}


void lex_string_literal_tests()
{
        init_stream("\"\" \"typedef\" \"enum\" \"struct\"");
        assert_token_empty_str();
        assert_token_str("typedef");
        assert_token_str("enum");
        assert_token_str("struct");
        assert_token_eof();
        
        init_stream("\"escaped chars\\n\\r\\tquotes \\'\\\"\\\\ null\\0\"");
        assert_token_str("escaped chars\n\r\tquotes \'\"\\ null\0");
        assert_token_eof();
}


void lex_char_literal_tests()
{
        init_stream("'a' 'b' 'c'");
        assert_token_int('a');
        assert_token_int('b');
        assert_token_int('c');
        assert_token_eof();
        
        init_stream("'\\0' '\\n' '\\r' '\\t' '\\v' '\\'' '\\\"' '\\\\'");
        assert_token_int(0);
        assert_token_int('\n');
        assert_token_int('\r');
        assert_token_int('\t');
        assert_token_int('\v');
        assert_token_int('\'');
        assert_token_int('\"');
        assert_token_int('\\');
        assert_token_eof();
}


void lex_basic_token_tests()
{
        init_stream("()[]{} ? : ; , . ! ~");
        assert_token(TOKEN_L_PAREN);
        assert_token(TOKEN_R_PAREN);
        assert_token(TOKEN_L_BRACE);
        assert_token(TOKEN_R_BRACE);
        assert_token(TOKEN_L_BRACKET);
        assert_token(TOKEN_R_BRACKET);
        assert_token(TOKEN_QUESTION);
        assert_token(TOKEN_COLON);
        assert_token(TOKEN_SEMICOLON);
        assert_token(TOKEN_COMMA);
        assert_token(TOKEN_DOT);
        assert_token(TOKEN_NOT);
        assert_token(TOKEN_NEG);
        assert_token_eof();
}


void lex_operator_tests()
{
        init_stream("= := * / % & << >> + - ^ | == < > <= >= != && ||");
        assert_token(TOKEN_ASSIGN);
        assert_token(TOKEN_COLON_ASSIGN);
        assert_token(TOKEN_MUL);
        assert_token(TOKEN_DIV);
        assert_token(TOKEN_MOD);
        assert_token(TOKEN_AND);
        assert_token(TOKEN_LSHIFT);
        assert_token(TOKEN_RSHIFT);
        assert_token(TOKEN_ADD);
        assert_token(TOKEN_SUB);
        assert_token(TOKEN_XOR);
        assert_token(TOKEN_OR);
        assert_token(TOKEN_EQ);
        assert_token(TOKEN_LT);
        assert_token(TOKEN_GT);
        assert_token(TOKEN_LTEQ);
        assert_token(TOKEN_GTEQ);
        assert_token(TOKEN_NEQ);
        assert_token(TOKEN_LOGICAL_AND);
        assert_token(TOKEN_LOGICAL_OR);
        assert_token_eof();
        
        init_stream("++ -- := *= /= %= &= <<= >>= += -= ^= |=");
        assert_token(TOKEN_INC);
        assert_token(TOKEN_DEC);
        assert_token(TOKEN_COLON_ASSIGN);
        assert_token(TOKEN_MUL_ASSIGN);
        assert_token(TOKEN_DIV_ASSIGN);
        assert_token(TOKEN_MOD_ASSIGN);
        assert_token(TOKEN_AND_ASSIGN);
        assert_token(TOKEN_LSHIFT_ASSIGN);
        assert_token(TOKEN_RSHIFT_ASSIGN);
        assert_token(TOKEN_ADD_ASSIGN);
        assert_token(TOKEN_SUB_ASSIGN);
        assert_token(TOKEN_XOR_ASSIGN);
        assert_token(TOKEN_OR_ASSIGN);
        assert_token_eof();
}


void lex_keyword_tests()
{
        init_stream("while my_name cast with const qualifier default kiril27");
        assert_token_keyword(while_keyword);
        assert_token_name("my_name");
        assert_token_keyword(cast_keyword);
        assert_token_name("with");
        assert_token_keyword(const_keyword);
        assert_token_name("qualifier");
        assert_token_keyword(default_keyword);
        assert_token_name("kiril27");
        assert_token_eof();
}


void lex_test()
{
        init_keywords();
        
        lex_integer_literal_tests();
        lex_float_literal_tests();
        lex_string_literal_tests();
        lex_char_literal_tests();
        lex_basic_token_tests();
        lex_operator_tests();
        lex_keyword_tests();
}

#endif

