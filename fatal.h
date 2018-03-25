#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


void fatal(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        
        printf("error: ");
        vprintf(fmt, args);
        printf("\n");
        
        va_end(args);
        exit(1);
}

#endif

