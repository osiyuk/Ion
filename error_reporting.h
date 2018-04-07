#ifndef ERROR_REPORTING
#define ERROR_REPORTING

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


void error(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        
        vprintf(fmt, args);
        printf("\n");
        
        va_end(args);
}

#define fatal_error(...) (error(__VA_ARGS__), exit(1))
#define syntax_error(...) (printf("syntax_error: "), error(__VA_ARGS__))

#endif

