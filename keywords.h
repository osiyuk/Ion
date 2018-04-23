#ifndef ION_KEYWORDS
#define ION_KEYWORDS

#include "stretchy_buffer.h"
#include "string_interning.h"


void init_keywords();
char is_keyword(const char *);

#define KEYWORD(name) \
const char *name##_keyword;
#include "keywords.txt"
#undef KEYWORD

const char **keywords;


void init_keywords()
{
        static char inited = 0;
        if (inited) {
                return;
        }
        keywords = buf_grow(NULL, 32, sizeof(char *));
#define KEYWORD(name) \
        name##_keyword = str_intern(#name); \
        buf__push(keywords, name##_keyword);
#include "keywords.txt"
#undef KEYWORD
        assert(str_intern("func") == func_keyword);
        inited = 1;
}


char is_keyword(const char *name)
{
        for (size_t i = 0; i < buf_len(keywords); i++) {
                if (keywords[i] == name)
                        return 1;
        }
        return 0;
}

#endif

