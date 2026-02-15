#ifndef AST_H
#define AST_H

typedef enum {
    AST_ID,
    AST_ADD,
    AST_ASSIGN
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *name;                 // for ID
    struct ASTNode *left;       // child 1
    struct ASTNode *right;      // child 2
} ASTNode;

/* constructors */
ASTNode* make_id(char *name);
ASTNode* make_add(ASTNode *l, ASTNode *r);
ASTNode* make_assign(ASTNode *l, ASTNode *r);

/* printer */
void print_ast(ASTNode *node, int indent);
void check_ast(ASTNode *node);

#endif

