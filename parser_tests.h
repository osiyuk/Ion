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
                "*x - *y",
                "3 - 2 - 1 - 0",
                "a * b - c << 1 & 3 ^ 4 | 5",
                "a || b && c == d && e < f | b ^ c & d >> 3 + 7 % 6",
                "(a & b == 0)",
                "(a == b || c == d)",
                "max.x - min.x",
                "n * fact_rec(n - 1)",
        };
        const char *ast[] = {
                "(* (++ p))",
                "([] \"sex\" 2)",
                "(- (* x) (* y))",
                "(- (- (- 3 2) 1) 0)",
                "(| (^ (& (<< (- (* a b) c) 1) 3) 4) 5)",
                "(|| a (&& b (&& (== c d) (< e (| f (^ b (& c (>> d (+ 3 (% 7 6))))))))))",
                "(== (& a b) 0)",
                "(|| (== a b) (== c d))",
                "(- (. max x) (. min x))",
                "(* n (fact_rec (- n 1)))",
        };
        size_t len = sizeof(expressions) / sizeof(char *);
        
        for (size_t i = 0; i < len; i++) {
                init_stream(expressions[i]);
                print_expr(parse_expr());
                test_print_buf(ast[i]);
        }
}


void parser_test()
{
        use_print_buf = YES;
        buf_init(print_buf);
        
        init_keywords();
        
        parser_expression_tests();
        
        use_print_buf = NO;
}

#endif

