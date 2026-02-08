#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* make_id(char *name) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = AST_ID;
    n->name = strdup(name);
    n->left = n->right = NULL;
    return n;
}

ASTNode* make_add(ASTNode *l, ASTNode *r) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = AST_ADD;
    n->left = l;
    n->right = r;
    n->name = NULL;
    return n;
}

ASTNode* make_assign(ASTNode *l, ASTNode *r) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = AST_ASSIGN;
    n->left = l;
    n->right = r;
    n->name = NULL;
    return n;
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++)
        printf(" ");

    switch (node->type) {
        case AST_ID:
            printf("ID(%s)\n", node->name);
            break;
        case AST_ADD:
            printf("ADD\n");
            break;
        case AST_ASSIGN:
            printf("ASSIGN\n");
            break;
    }

    print_ast(node->left, indent + 2);
    print_ast(node->right, indent + 2);
}

