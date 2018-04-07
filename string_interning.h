#ifndef STRING_INTERNING
#define STRING_INTERNING

// https://en.wikipedia.org/wiki/String_interning

#include <assert.h>
#include <string.h>
#include "stretchy_buffer.h"

struct row_t {
        size_t len;
        const char *str;
};

static struct row_t *table = NULL;


const char *str_intern_slice(const char *str, size_t len)
{
        struct row_t *row;
        
        if (table == NULL)
                goto push;
        
        for (int i = 0; i < buf_len(table); i++) {
                row = table + i;
                if (row->len == len && strncmp(row->str, str, len) == 0)
                        return row->str;
        }
        
push:
        str = strndup(str, len);
        buf_push(table, ((struct row_t) {len, str}));
        return str;
}


const char *str_intern(const char *str)
{
        return str_intern_slice(str, strlen(str));
}


void str_test()
{
        const char *px, *py, *pz;
        char x[] = "hello";
        char y[] = "hello";
        char z[] = "hello!";
        assert(x != y);
        px = str_intern(x);
        py = str_intern(y);
        assert(px == py);
        pz = str_intern(z);
        assert(pz != px);
}

#endif

