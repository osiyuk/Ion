#ifndef ERROR_REPORTING
#define ERROR_REPORTING


void error(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        
        vprintf(fmt, args);
        printf("\n");
        
        va_end(args);
}

#define log_error(...) (printf("error: "), error(__VA_ARGS__))
#define syntax_error(...) (printf("syntax_error: "), error(__VA_ARGS__))
#define fatal_error(...) (log_error(__VA_ARGS__), exit(1))

#endif

