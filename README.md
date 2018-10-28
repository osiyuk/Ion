# Ion: A C-Like Systems Programming Language

Goals
- Easy to implement
- Immediately familiar to C programmers, no learning curve
- Reduce distractions, needless features
- Comfortable for day to day work, dogfooding
- Usable for Linux x86_64 host development
- For host, interop seamlessly with C and host OS
- For target, generate machine code, don't need C compatibility
- Protect long-term code investment
- Eventually usable for other hosts (Windows/Linux/Mac x86/x64/ARM) and targets (RISC-V) development

Non-goals
- "Best language ever" syndrome
- Highly opinionated vs C
- Raising level of abstraction
- Memory/integer overflow safety

Why C?
- Appropriate level of abstraction
- Familiar to target audience
- Ecosystem, libraries, toolchain
- Code is protected investment

But why not C?
- Needless compiler complexity, distracting gotchas, legacy cruft
- Challenging to build good non-compiler tooling
- Parsing requires symbol table, large/unbounded lookahead
- Type designator syntax too complex
- Broken precedence table
- Implicit arithmetic conversions/integer promotions, notoriously bug prone
- Overuse of casts vs explicit conversion/construction
- No packages/modules, overreliance on preprocessor, physical coupling
- Undefined behavior everywhere (signed overflow, illegal aliasing, shifts, etc)

Resolution
- Use the C type system, machine model, platform ABIs, etc
- Simple C-like syntax: LL(1), independent of symbol table
- Converged to similar changes to C made by Go and Swift, but unabashedly low level, no GC, etc
- For host, emit machine code directly, start with Wirth-style one-pass code gen
- Easy to bootstrap, interop, portable (modulo ABI), toolchain for free (no PDB hell), #line magic
- Generates idiomatic C packages, protects code investment
- Unfortunately stuck with some undefined behavior from C :(

Quality of life
- Order-independent functions, bonus from 2-pass
- Run-time introspection
- Fast, powerful non-compiler tooling, powered by compiler and introspection
- Packages, convention over configuration, relies on order-independent functions

Implementation
- Bootstrap as C99 codebase
- Eventually convert to self-hosting Ion codebase to run on target
- Simple 2-pass compiler
- Pass 1, lex, LL(1) parse, produces AST
- Pass 2, resolve, type check, code gen, produces x86_64 machine code/metadata


Let's look at some [example code](example.ion).
