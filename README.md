# Recursive Descent Math Parser (C)

A lightweight mathematical expression evaluator that builds an Abstract Syntax Tree (AST) to solve functions of x.

## Features
- **Recursive Descent Parsing**: Implements a formal grammar for mathematical expressions.
- **Operator Precedence**: Correct handling of PEMDAS (Parentheses, Exponents, Multiplication/Division, Addition/Subtraction).
- **Right-Associativity**: Correctly evaluates exponents (e.g., `2^3^2` as `2^(3^2)`).
- **AST Evaluation**: Separates the parsing logic from the evaluation logic for efficiency.

## Grammar Rules
The parser follows these EBNF rules:
```ebnf
expression : term ((+|-) term)*
term       : power ((*|/) power)*
power      : factor (^ power)
factor     : NUMBER | VAR | ( expression ) | - factor