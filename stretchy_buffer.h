#ifndef STRETCHY_BUFFERS
#define STRETCHY_BUFFERS

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
        size_t len;
        size_t cap;
        char buf[0];
} Bufhdr;

#define buf__hdr(b) ((Bufhdr *) ((size_t *) b - 2))
#define buf__len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf__cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf__fits(b, n) buf__len(b) + (n) <= buf__cap(b)
#define buf__grow(b, n) buf_grow((b), buf__len(b) + (n), sizeof(*(b)))
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((b) = buf__grow(b, n)))
#define buf__push(b, x) (b[buf_len(b)++] = (x))

#define buf_init(b) buf__fit(b, 4)
#define buf_len(b) buf__hdr(b)->len
#define buf_push(b, x) (buf__fit(b, 1), b[buf_len(b)++] = (x))
#define buf_pop(b) (b[--buf_len(b)])
#define buf_top(b) (b[buf_len(b) - 1])

void *buf_grow(const void *buf, size_t len, size_t elem_size)
{
        size_t new_cap, new_size;
        Bufhdr *hdr;
        
        new_cap = buf__cap(buf) ? 2 * buf__cap(buf) : len;
        assert(len <= new_cap);
        new_size = offsetof(Bufhdr, buf) + new_cap * elem_size;
        
        if (buf) {
                hdr = realloc(buf__hdr(buf), new_size);
        } else {
                hdr = malloc(new_size);
                hdr->len = 0;
        }
        hdr->cap = new_cap;
        return hdr->buf;
}


void buf_test()
{
        int *ints = NULL;
        enum { N = 1024 };
        for (int i = 0; i < N; i++) {
                buf_push(ints, i);
        }
        assert(buf_len(ints) == N);
        for (int i = 0; i < (int) buf_len(ints); i++) {
                assert(ints[i] == i);
        }
}

#endif

