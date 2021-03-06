#ifndef STRING_INTERNING
#define STRING_INTERNING


struct _string {
        size_t len;
        const char *str;
};

static struct _string *table = NULL;


const char *str_intern_slice(const char *str, size_t len)
{
        struct _string *s;
        
        if (table == NULL)
                goto push;
        
        for (int i = 0; i < buf_len(table); i++) {
                s = table + i;
                if (s->len == len && strncmp(s->str, str, len) == 0)
                        return s->str;
        }
        
push:
        str = strndup(str, len);
        buf_push(table, ((struct _string) {len, str}));
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

