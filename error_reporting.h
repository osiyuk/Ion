#ifndef ERROR_REPORTING
#define ERROR_REPORTING


const char *filename;
int line_number;


void error(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        
        vprintf(fmt, args);
        printf("\n");
        
        va_end(args);
}

#define header(type) printf("%s:%d %s: ", filename, line_number, type)
#define log_error(...) (printf("error: "), error(__VA_ARGS__))
#define syntax_error(...) (header("error"), error(__VA_ARGS__))
#define fatal_error(...) (log_error(__VA_ARGS__), exit(1))

#endif

