#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Structs and Enums

typedef enum {
    T_NUMBER,
    T_VARIABLE,
    T_LPAR,
    T_RPAR,
    T_EXP,
    T_MUL,
    T_DIV,
    T_PLUS,
    T_MINUS,
    T_END
 } tokenType;

typedef struct Token {
    tokenType type;
    double value;
    char varName;
} Token;

typedef struct Parser {
    Token currentToken;
    struct 
    {
        char *string;
        int pos;
    } lexer;
} Parser;

typedef enum {
    N_NUMBER,
    N_VAR,
    N_OP
} nodeType;

typedef struct ASTnode {
    nodeType type;
    union {
        double value;
        char op;
    } data;
    struct ASTnode *left;
    struct ASTnode *right;
} ASTnode;

// Function prototypes 

Parser* parser_init(char *string);
Token tokenizer(Parser *p);
void eat(Parser *p, tokenType expected);
ASTnode* create_number_node(double number);
ASTnode* create_op_node(char op, ASTnode* l, ASTnode* r);
ASTnode* factor(Parser *p);
ASTnode* term(Parser *p);
ASTnode* expression(Parser *p);
double evaluate_tree(ASTnode* n, double x_val);
void free_tree(ASTnode *n);

// Parser initialization

Parser* parser_init(char *string) {
    Parser *parser = malloc(sizeof(Parser));
    if (!parser) { return 0; }
    parser->lexer.string = string;
    parser->lexer.pos = 0;
    parser->currentToken = tokenizer(parser);
    return parser;
}

// Lexical analysis

Token tokenizer(Parser *p) {
    while (p->lexer.string[p->lexer.pos] == ' ') {
        p->lexer.pos++;
    }

    if (p->lexer.string[p->lexer.pos] == '\0') {
        return (Token) {T_END, 0};
    }

    char c = p->lexer.string[p->lexer.pos];

    if (c == 'x') {
        p->lexer.pos++;
        return (Token) {T_VARIABLE, 0, c}; // make default x_val = 0;
    }

    // Handle numbers (including multi-digits and decimals)
    if (c >= '0' && c <= '9' || c == '.') {
        char *startPtr = &p->lexer.string[p->lexer.pos];
        char *endPtr;
        double val = strtod(startPtr, &endPtr);
        // Move the position forward by the number of characters strtod read
        p->lexer.pos += (endPtr - startPtr);
        return (Token) {T_NUMBER, val};
    }

    p->lexer.pos++;

    // Handle operators and parentheses
    switch (c) {
        case '(': return (Token) {T_LPAR, 0};
        case ')': return (Token) {T_RPAR, 0};
        case '*': return (Token) {T_MUL, 0};
        case '/': return (Token) {T_DIV, 0};
        case '^': return (Token) {T_EXP, 0};
        case '+': return (Token) {T_PLUS, 0};
        case '-': return (Token) {T_MINUS, 0};
        default:
            printf("Syntax error, unknown character %c\n", c);
            exit(1);
    }


}

void eat(Parser *p, tokenType expected) {
    if (p->currentToken.type == expected) {
        p->currentToken = tokenizer(p);
    }
    else {
        printf("Unexpected token!");
        exit(1);
    }
}

// ASTnode constructors

ASTnode* create_number_node(double number) {
    ASTnode* n = malloc(sizeof(ASTnode));
    if (!n) return NULL;
    n->type = N_NUMBER;
    n->data.value = number;
    n->left = NULL;
    n->right = NULL;
    return n;
}

ASTnode* create_var_node(char var_name) {
    ASTnode *n = malloc(sizeof(ASTnode));
    if (!n) return NULL;
    n->type = N_VAR;
    n->data.op = var_name;
    n->left = NULL;
    n->right = NULL;
    return n;
}

ASTnode* create_op_node(char op, ASTnode* l, ASTnode* r) {
    ASTnode* n = malloc(sizeof(ASTnode));
    if (!n) return NULL;
    n->type = N_OP;
    n->data.op = op;
    n->left = l;
    n->right = r;
    return n;
}



ASTnode *factor(Parser *p) {
    if (p->currentToken.type == T_NUMBER) {
        double val = p->currentToken.value;
        eat(p, T_NUMBER);
        return create_number_node(val);
    }

    else if (p->currentToken.type == T_VARIABLE) {
        char name = p->currentToken.varName; // store name  
        eat(p, T_VARIABLE); // move to next token
        return create_var_node(name);
    }   

    else if (p->currentToken.type == T_LPAR) {
        eat(p, T_LPAR);
        ASTnode *node = expression(p); // Get subtree
        eat(p, T_RPAR);
        return node;
    }
    // Check for unary minus
    else if (p->currentToken.type == T_MINUS) {
        eat(p, T_MINUS);
        return create_op_node('-', NULL, factor(p));
    }

    else {
        printf("Unexpected token in factor()");
        exit(1);
    }
}

ASTnode* power(Parser *p) {
    ASTnode *node = factor(p);
    while (p->currentToken.type == T_EXP) {
        eat(p, T_EXP);
        node = create_op_node('^', node, power(p));
    }
    return node;
}

ASTnode* term(Parser *p) {
    ASTnode *node = power(p); // number node before op
    while (p->currentToken.type == T_MUL || p->currentToken.type == T_DIV) {
        char op = (p->currentToken.type == T_MUL) ? '*' : '/';
        eat(p, p->currentToken.type); // Move to the next token
        node = create_op_node(op, node, power(p)); // power(p) is number node after op 
    }
    return node;
}

ASTnode* expression(Parser *p) {
    ASTnode *node = term(p);
    while (p->currentToken.type == T_PLUS || p->currentToken.type == T_MINUS) {
        char op = (p->currentToken.type == T_PLUS) ? '+' : '-';
        eat(p, p->currentToken.type);
        node = create_op_node(op, node, term(p));
    }
    return node;

}
// Evaluate a tree by receiving the root node
double evaluate_tree(ASTnode* n, double x_val) {
    if (n == NULL) return 0; 

    else if (n->type == N_NUMBER) return n->data.value;
    
    else if (n->type == N_VAR) {
        if(n->data.op == 'x') {return x_val;}
        return 0.0;
    }

    // Recursive calls happen here
    double left = evaluate_tree(n->left, x_val);
    double right = evaluate_tree(n->right, x_val);

    switch(n->data.op) {
        case '+': return left + right;
        case '-': return left - right;
        case '*': return left * right;
        case '/':
            if(right == 0) { printf("Error! Division by zero!\n"); exit(1); }
            return left / right;
        case '^': return pow(left, right);
        default: return 0;
    }
}

// Memory management

void free_tree(ASTnode* n) {
    if (!n) return;
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}


int main() {
    char toCalc[256];
    double x_val;
    printf("Enter a function of x: ");
    fgets(toCalc, sizeof(toCalc), stdin);
    toCalc[strcspn(toCalc, "\n")] = 0; // Remove newline

    printf("Enter value for x: ");
    scanf("%lf", &x_val);

    // Initialize parser and build AST
    Parser* p = parser_init(toCalc);
    ASTnode* tree = expression(p);

    double result = evaluate_tree(tree, x_val);
    printf("Result: %.2f\n", result);

    free_tree(tree);
    free(p);
    return 0;
}