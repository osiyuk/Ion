#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE


enum SymKind {
        SYM_NONE,
        SYM_TYPE,
        SYM_CONST,
        SYM_ENUM_CONST,
        SYM_VAR,
        SYM_FUNC,
};

enum SymState {
        SYM_UNRESOLVED,
        SYM_RESOLVING,
        SYM_RESOLVED,
};

struct Sym {
        enum SymKind kind;
        enum SymState state;
        const char *name;
        union {
                Decl *decl;
                Type *type;
                int64_t val;
        };
};


enum {
        MAX_LOCAL_SYMBOLS = 32
};

Sym **global_symbols;
Sym local_symbols[MAX_LOCAL_SYMBOLS];

Sym *local_sym_stack_top = local_symbols;
#define PUSH(symbol) *local_sym_stack_top++ = symbol
#define STACK_SIZE() (local_sym_stack_top - local_symbols)


void *sym_alloc(int size)
{
        void *ptr;
        
        assert(size > 0);
        ptr = malloc(size);
        memset(ptr, 0, size);
        return ptr;
}


Sym *new_sym(enum SymKind kind, const char *name, Decl *decl)
{
        Sym *s = sym_alloc(sizeof(Sym));
        s->kind = kind;
        s->state = SYM_UNRESOLVED;
        s->name = name;
        s->decl = decl;
        return s;
}


Sym *new_sym_type(const char *name, Type *type)
{
        Sym *s = new_sym(SYM_TYPE, name, NULL);
        s->state = SYM_RESOLVED;
        s->type = type;
        return s;
}


Sym *new_sym_enum_const(const char *name, Decl *decl)
{
        return new_sym(SYM_ENUM_CONST, name, decl);
}


Sym *new_sym_from_decl(Decl *decl)
{
        enum SymKind kind = SYM_NONE;
        Sym *s;
        
        switch (decl->kind) {
        case DECL_TYPEDEF:
        case DECL_ENUM:
        case DECL_STRUCT:
        case DECL_UNION:
                kind = SYM_TYPE;
                break;
        case DECL_CONST:
                kind = SYM_CONST;
                break;
        case DECL_VAR:
                kind = SYM_VAR;
                break;
        case DECL_FUNC:
                kind = SYM_FUNC;
                break;
        default:
                assert(DECL_NONE);
        }
        
        s = new_sym(kind, decl->name, decl);
        switch (decl->kind) {
        case DECL_STRUCT:
        case DECL_UNION:
                s->state = SYM_RESOLVED;
                s->type = type_incomplete(s);
        default:
                break;
        }
        return s;
}


Sym *sym_global_type(const char *name, Type *type)
{
        Sym *symbol = new_sym_type(name, type);
        buf_push(global_symbols, symbol);
        return symbol;
}


Sym *sym_global_decl(Decl *decl)
{
        Sym *symbol, *enum_const;
        size_t num_names;
        const char **names;
        
        symbol = new_sym_from_decl(decl);
        buf_push(global_symbols, symbol);
        if (decl->kind != DECL_ENUM) {
                return symbol;
        }
        num_names = decl->enum_decl.num_names;
        names = decl->enum_decl.names;
        
        for (size_t i = 0; i < num_names; i++) {
                enum_const = new_sym_enum_const(names[i], decl);
                buf_push(global_symbols, enum_const);
        }
        return symbol;
}


Sym *sym_enter()
{
        return local_sym_stack_top;
}


void sym_leave(Sym *top)
{
        local_sym_stack_top = top;
}


void sym_push(const char *name, Type *type)
{
        Sym symbol;
        
        if (local_sym_stack_top == local_symbols + MAX_LOCAL_SYMBOLS) {
                fatal_error("Local symbols stack overflow");
        }
        symbol.kind = SYM_VAR;
        symbol.state = SYM_RESOLVED;
        symbol.name = name;
        symbol.type = type;
        PUSH(symbol);
}


Sym *sym_get(const char *name)
{
        Sym *symbol;
        
        for (size_t i = STACK_SIZE(); i > 0; i--) {
                symbol = local_symbols + i - 1;
                if (symbol->name == name)
                        return symbol;
        }
        for (size_t i = 0; i < buf_len(global_symbols); i++) {
                symbol = global_symbols[i];
                if (symbol->name == name)
                        return symbol;
        }
        return NULL;
}

#undef STACK_SIZE
#undef PUSH
#endif

