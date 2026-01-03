# Recursive Descent Math Parser (C)

A lightweight mathematical expression evaluator that builds an Abstract Syntax Tree (AST) to solve functions of x.

## Features
- **Recursive Descent Parsing**: Implements a formal grammar for mathematical expressions.
- **Operator Precedence**: Correct handling of PEMDAS (Parentheses, Exponents, Multiplication/Division, Addition/Subtraction).
- **Encapsulated State**: Uses a `Parser` struct to manage state instead of global variables, providing better encapsulation and memory safety.Previous version used global token variable.
- **Right-Associativity**: Correctly evaluates exponents (e.g., `2^3^2` as `2^(3^2)`).
- **AST Evaluation**: Separates the parsing logic from the evaluation logic for efficiency.

## Grammar Rules
The parser follows these EBNF rules:
```text
expression : term ((+|-) term)*
term       : power ((*|/) power)*
power      : factor (^ power)
factor     : NUMBER | VAR | ( expression ) | - factor
``` 

## How to Run
``` markdown
1. Open your terminal
2. Run: gcc main.c -o parser -lm 
3. Run: ./parser (or parser.exe on Windows)
```

## Example Usage
``` text
Enter a function of x: (-5 + x) * 2 ^ (10 / 5)
Enter value for x: 8
Result: 12.00
```


