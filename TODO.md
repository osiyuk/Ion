# My plan for doing things

Basic pieces of compiler
- Parser error recovery
- Resolve ast
- Type checking
- Hash functions
- Hash table (hash map)
- Arena allocator
- Visual structure of errors
- Thread safe lexer
- Compound literals
- Easy compiler profiling
- Unnamed structs/unions/enums
- Support bitfields?

What to do next?
- AST: Move assign operators from statement to expression level
- AST: Eliminate StmtList structure for STMT_FOR
- Implement simple Wirth-style error recovering strategy
- Introduce brand new parser
- Watch bitwise day 14: Type revisited
