#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum tokenType {
    T_NUMBER,
    T_VAR,
    T_PLUS,
    T_MINUS,
    T_MUL,
    T_DIV,
    T_EXP,
    T_LPAR,
    T_RPAR,
    T_END
} tokenType;

typedef struct Token {
    tokenType type;
    double value;
} Token;

Token currentToken; 

typedef struct Lexer {
    char *text;
    int pos;
} Lexer;

Lexer* lexer_init(char *string) {
    Lexer *l = malloc(sizeof(Lexer));
    l->text = string;
    l->pos = 0;
    return l;
}

Token tokenizer(Lexer *lexer) {
    
    if (lexer->text[lexer->pos] == '\0') {
        return (Token) { T_END, 0};
    }

    while (lexer->text[lexer->pos] == ' ') {
        lexer->pos++;
    }

    char c = lexer->text[lexer->pos];

    if ( (c >= '0' && c <= '9') || c == '.') {
        char *endPtr;
        
        double val = strtod(&lexer->text[lexer->pos], &endPtr); // strtod reads the number and 'endPtr' tells us where the number ended
    
        
        lexer->pos += (endPtr - &lexer->text[lexer->pos]); // We move the lexer's position forward by however many characters strtod read
    
        return (Token){T_NUMBER, val};
    }

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        lexer->pos++;
        return (Token){T_VAR, (double)c};   // cast ascii to double
    }

    lexer->pos++;
    switch (c) {
        case '+': return (Token){T_PLUS, 0};
        case '-': return (Token){T_MINUS, 0};
        case '*': return (Token){T_MUL, 0};
        case '/': return (Token){T_DIV, 0};
        case '^': return (Token){T_EXP, 0};
        case '(': return (Token){T_LPAR, 0};
        case ')': return (Token){T_RPAR, 0};
        default: 
            printf("Unexpected character: %c\n", c);
            exit(1);
    }
} 


void eat(Lexer *l, tokenType expected) {

    if (currentToken.type == expected) {
        currentToken = tokenizer(l);
    }
    else {
        printf("Syntax Error, expected %d but got %d\n", expected, currentToken.type);
        exit(1);
    }

}

typedef enum ASTnodeType {
    N_NUMBER,
    N_VAR,
    N_OP
} ASTnodeType;

/* The following parser implements Recursive Descent based on the following grammar:
 * expression : term ((+|-) term)* read as -> expression is term  (plus or minus term), zero or more times
 * term       : power ((*|/) power)*
 * power      : factor (^ power)
 * factor     : NUMBER | VAR | ( expression ) | - factor
 */

typedef struct ASTnode {
    ASTnodeType type;
    union {
        double number;
        char var_name; // X or Y or ..
        struct {
            char c;
            struct ASTnode *left;
            struct ASTnode *right;
        } op;
    } data;

} ASTnode ;

// Forward declarations

ASTnode* expression(Lexer *l);
ASTnode* term(Lexer *l);
ASTnode* power(Lexer *l);
ASTnode* factor(Lexer *l);

// Constructors
// Program is small enough to trust malloc and not NULL check

ASTnode* create_number_node(double number) {
    ASTnode *node = malloc(sizeof(ASTnode));
    node->type = N_NUMBER;
    node->data.number = number;
    return node;
}

ASTnode* create_var_node(char var_name) {
    ASTnode *node = malloc(sizeof(ASTnode));
    node->type = N_VAR;
    node->data.var_name = var_name;
    return node;
}

ASTnode* create_op_node(char operator, ASTnode *left, ASTnode *right) {
    ASTnode *node = malloc(sizeof(ASTnode));
    node->type = N_OP;
    node->data.op.c = operator;
    node->data.op.left = left;
    node->data.op.right = right;
    return node;
}

// Grammar Rules

ASTnode* factor(Lexer *l) {
    if (currentToken.type == T_NUMBER) {
        double val = currentToken.value;
        eat(l, T_NUMBER);
        return create_number_node(val);
    }

    if (currentToken.type ==  T_VAR) {
        char name = (char) currentToken.value;
        eat(l, T_VAR);
        return create_var_node(name);
    }

    else if (currentToken.type == T_LPAR) {
        eat(l, T_LPAR);
        ASTnode *node = expression(l); 
        eat(l, T_RPAR);
        return node;     
    }

    else if (currentToken.type == T_MINUS) {
        eat(l, T_MINUS);
        // Unary minus: treat -x as 0 - x
        return create_op_node('-', create_number_node(0), factor(l));
    }

    printf("Syntax Error, unexpected token %d\n", currentToken.type);
    exit(1);
    return NULL; 
}

ASTnode* power(Lexer *l) {
    ASTnode* node = factor(l);
    while (currentToken.type == T_EXP) {
        eat(l, T_EXP);
        // Right associative recursion
        node = create_op_node('^', node, power(l));
    }
    return node;
}

ASTnode* term(Lexer *l) {
    ASTnode *node = power(l); 
    while (currentToken.type == T_MUL || currentToken.type == T_DIV) {
        char op = (currentToken.type == T_MUL) ? '*' : '/';
        eat(l, currentToken.type);
        node = create_op_node(op, node, power(l));
    }
    return node;
}

ASTnode* expression(Lexer *l) {
    ASTnode *node = term(l);
    while (currentToken.type == T_PLUS || currentToken.type == T_MINUS) {
        char op = (currentToken.type == T_PLUS) ? '+' : '-';
        eat(l, currentToken.type);
        node = create_op_node(op, node, term(l));
    }
    return node;
}

double evaluate(ASTnode* node, double x_var) {
    if (node->type == N_NUMBER) {
        return node->data.number;
    }

    if (node->type == N_VAR) {
        if (node->data.var_name == 'x') return x_var;
        return 0.0;
    }

    double left = evaluate(node->data.op.left, x_var);
    double right = evaluate(node->data.op.right, x_var);

    if (node->data.op.c == '+') return left + right;
    else if (node->data.op.c == '-') return left - right;
    else if (node->data.op.c == '*') return left * right;
    else if (node->data.op.c == '/') return left / right;
    else if (node->data.op.c == '^') return pow(left, right);

    return 0.0;
}

void free_tree(ASTnode* node) {
    if (node == NULL) return;
    if (node->type == N_OP) {
        free_tree(node->data.op.left);
        free_tree(node->data.op.right);
    }
    free(node);
}





int main () {
    char input[256];
    double x_val;

    printf("Enter a function of x: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0; // Remove newline

    printf("Enter value for x: ");
    scanf("%lf", &x_val);

    Lexer* l = lexer_init(input);
    currentToken = tokenizer(l);
    ASTnode* tree = expression(l);

    double result = evaluate(tree, x_val);
    printf("Result: %.2f\n", result);

    free_tree(tree);
    free(l);
    return 0;
}