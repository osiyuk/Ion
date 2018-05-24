#ifndef TYPE_SYSTEM
#define TYPE_SYSTEM


typedef struct Sym Sym;
typedef struct Type Type;


enum TypeKind {
        TYPE_NONE,
        TYPE_INCOMPLETE,
        TYPE_COMPLETING,
        TYPE_VOID,
        TYPE_CHAR,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_PTR,
        TYPE_ARRAY,
        TYPE_ENUM,
        TYPE_STRUCT,
        TYPE_UNION,
        TYPE_FUNC,
};

struct Type {
        enum TypeKind kind;
        size_t size;
        size_t align;
        union {
                Sym *symbol;
                Type *elem;
                
                struct {
                        Type *type;
                        size_t length;
                } array;
                
                struct {
                        const char **fields;
                        Type **types;
                        size_t num_fields;
                } bucket;
                
                struct {
                        Type **args;
                        size_t num_args;
                        Type *ret;
                } func;
        };
};


const size_t PTR_SIZE = 8;
const size_t PTR_ALIGN = 8;

Type **cached_ptr_types; // The whole idea about caching types
Type **cached_array_types; // is to compare types by pointers
Type **cached_function_types;


void *type_alloc(size_t size)
{
        Type *ptr;
        
        assert(size > 0);
        ptr = malloc(size);
        memset(ptr, 0, size);
        return ptr;
}


Type *new_type(enum TypeKind kind, size_t size, size_t align)
{
        Type *t = type_alloc(sizeof(Type));
        t->kind = kind;
        t->size = size;
        t->align = align;
        return t;
}


Type *new_ptr(Type *elem)
{
        Type *t = new_type(TYPE_PTR, PTR_SIZE, PTR_ALIGN);
        t->elem = elem;
        return t;
}


Type *new_array(Type *type, size_t length)
{
        size_t size = type->size * length;
        Type *a = new_type(TYPE_ARRAY, size, type->align);
        a->array.type = type;
        a->array.length = length;
        return a;
}


Type *new_bucket(const char **fields, Type **types, size_t num_fields)
{
        Type *b = type_alloc(sizeof(Type));
        b->bucket.fields = fields;
        b->bucket.types = types;
        b->bucket.num_fields = num_fields;
        return b;
}


Type *new_func(Type **args, size_t num_args, Type *ret)
{
        Type *f = new_type(TYPE_FUNC, PTR_SIZE, PTR_ALIGN);
        f->func.ret = ret;
        f->func.args = args;
        f->func.num_args = num_args;
        return f;
}


Type *type_incomplete(Sym *symbol)
{
        Type *t = new_type(TYPE_INCOMPLETE, 0, 0);
        t->symbol = symbol;
        return t;
}


Type *type_ptr(Type *elem)
{
        Type *cached, *type;
        
        for (size_t i = 0; i < buf_len(cached_ptr_types); i++) {
                cached = cached_ptr_types[i];
                if (cached->elem == elem)
                        return cached;
        }
        type = new_ptr(elem);
        buf_push(cached_ptr_types, type);
        return type;
}


Type *type_array(Type *type, size_t length)
{
        Type *cached;
        
        for (size_t i = 0; i < buf_len(cached_array_types); i++) {
                cached = cached_array_types[i];
                if (    cached->array.type == type &&
                        cached->array.length == length) {
                                return cached;
                }
        }
        type = new_array(type, length);
        buf_push(cached_array_types, type);
        return type;
}


Type *type_func(Type **args, size_t num_args, Type *ret)
{
        Type *cached, *type;
        
        for (size_t i = 0; i < buf_len(cached_function_types); i++) {
                char match = TRUE;
                cached = cached_function_types[i];
                if (    cached->func.num_args != num_args ||
                        cached->func.ret != ret) continue;
                for (size_t j = 0; j < num_args; j++) {
                        if (cached->func.args[j] != args[j]) {
                                match = FALSE;
                                break;
                        }
                }
                if (match)
                        return cached;
        }
        type = new_func(args, num_args, ret);
        buf_push(cached_function_types, type);
        return type;
}

#endif

