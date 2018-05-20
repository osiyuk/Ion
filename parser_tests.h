#ifndef PARSER_REGRESSION_TESTS
#define PARSER_REGRESSION_TESTS


enum {
        NO,
        YES
};


void test_print_buf(const char *ast)
{
        if (strcmp(print_buf, ast)) { log_error(
                "parser: expected ast\n"
                "%s\n"
                "got\n"
                "%s", ast, print_buf);
        }
        buf_len(print_buf) = 0;
}


void parser_expression_tests()
{
        const char *expressions[] = {
                "*p++",
                "\"sex\"[2]",
                "x || y || z",
                "*x - *y",
                "3 - 2 - 1 - 0",
                "2 * 3 + 4 * 5",
                "2 * (3 + 4) * 5",
                "a * b - c << 1 & 3 ^ 4 | 5",
                "a || b && c == d && e < f | b ^ c & d >> 3 + 7 % 6",
                "(a & b == 0)",
                "(a == b && c == d)",
                "max.x - min.x",
                "n * fact_rec(n - 1)",
        };
        const char *ast[] = {
                "(* (++ p))",
                "([] \"sex\" 2)",
                "(|| (|| x y) z)",
                "(- (* x) (* y))",
                "(- (- (- 3 2) 1) 0)",
                "(+ (* 2 3) (* 4 5))",
                "(* (* 2 (+ 3 4)) 5)",
                "(| (^ (& (<< (- (* a b) c) 1) 3) 4) 5)",
                "(|| a (&& b (&& (== c d) (< e (| f (^ b (& c (>> d (+ 3 (% 7 6))))))))))",
                "(== (& a b) 0)",
                "(&& (== a b) (== c d))",
                "(- (. max x) (. min x))",
                "(* n (call fact_rec (- n 1)))",
        };
        size_t len = sizeof(expressions) / sizeof(char *);
        
        for (size_t i = 0; i < len; i++) {
                init_stream(expressions[i]);
                print_expr(parse_expr());
                test_print_buf(ast[i]);
        }
}


void parser_typespec_tests()
{
        const char *types[] = {
                "uint64_t",
                "int const",
                "char *",
                "char **",
                "int[64]",
                "func()",
                "func(): void",
                "func(): int[16]",
                "(func():int)[16]",
                "func(int): char *",
                "func (char *, int, float): char *",
                "func(uint8_t[3], uint8_t[3]): uint8_t[3]",
        };
        const char *ast[] = {
                "uint64_t",
                "(const int)",
                "(ptr char)",
                "(ptr (ptr char))",
                "(array int 64)",
                "(func () void)",
                "(func () void)",
                "(func () (array int 16))",
                "(array (func () int) 16)",
                "(func (int) (ptr char))",
                "(func ((ptr char), int, float) (ptr char))",
                "(func ((array uint8_t 3), (array uint8_t 3)) (array uint8_t 3))",
        };
        size_t len = sizeof(types) / sizeof(char *);
        
        for (size_t i = 0; i < len; i++) {
                init_stream(types[i]);
                print_type(parse_type());
                test_print_buf(ast[i]);
        }
}


void parser_statement_tests()
{
        const char *statements[] = {
                "{}",
                "{ t := a; a = b; b = t }",
                "a = b = c = x",
                "break",
                "continue",
                "return 13",
                "return;",
                "if (cond) print()",
                "if (cond) print() else sneak()",
                "while (compute()) {}",
                "while (compute()) ;",
                "while (cond) { print(hello); cond-- }",
                "do { sneak() } while (!cond)",
                "for (;;) sneak()",
                "for (; cond;) { print(); break; }",
                "for (i := 0; i < len; i++) printf(\"step \", i)",
                "switch (token.kind) { case NONE: break; " // 1
                "case TOK: printf(token.name); break; " // 2
                "default: printf(\"error\") }", // 3
        };
        const char *ast[] = {
                "(block nil)",
                "(block (:= t a) (= a b) (= b t))",
                "(= a (= b (= c x)))",
                "(break)",
                "(continue)",
                "(return 13)",
                "(return)",
                "(if cond (call print))",
                "(if cond (call print) else (call sneak))",
                "(while (call compute) (block nil))",
                "(while (call compute) (block nil))",
                "(while cond (block (call print hello) (-- cond)))",
                "(do (block (call sneak)) while (! cond))",
                "(for () () () (call sneak))",
                "(for () cond () (block (call print) (break)))",
                "(for (:= i 0) (< i len) (++ i) (call printf \"step \" i))",
                "(switch (. token kind) (case NONE (break)) " // 1
                "(case TOK (block (call printf (. token name)) " // 2
                "(break))) (default (call printf \"error\")))", // 3
        };
        size_t len = sizeof(ast) / sizeof(char *);
        
        for (size_t i = 0; i < len; i++) {
                init_stream(statements[i]);
                print_stmt(parse_statement());
                test_print_buf(ast[i]);
        }
}


void parser_declaration_tests()
{
        const char *declarations[] = {
                "typedef ptr = void *",
        };
        const char *ast[] = {
                "(typedef ptr (ptr void))",
        };
        size_t len = sizeof(ast) / sizeof(char *);
        
        for (size_t i = 0; i < len; i++) {
                init_stream(declarations[i]);
                print_decl(parse_declaration());
                test_print_buf(ast[i]);
        }
}


void parser_test()
{
        use_print_buf = YES;
        buf_init(print_buf);
        
        init_keywords();
        
        parser_expression_tests();
        parser_typespec_tests();
        parser_statement_tests();
        parser_declaration_tests();
        
        use_print_buf = NO;
}

#endif

