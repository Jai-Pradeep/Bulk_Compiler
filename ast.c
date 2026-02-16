#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

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


void check_ast(ASTNode *n) {
    if (!n) return;

    if (n->type == AST_ID) {
        Symbol *s = lookup_symbol(n->name);
        if (!s) {
            printf("Error: undeclared variable %s\n", n->name);
            exit(1);
        }
    }

    if (n->type == AST_ARRAY_ADD) {
        // Both children must be arrays of same size
        if (n->left && n->right) {
            Symbol *l = lookup_symbol(n->left->name);
            Symbol *r = lookup_symbol(n->right->name);
            if (!l || !r) {
                printf("Error: undeclared variable in array addition\n");
                exit(1);
            }
            if (l->type != SYM_ARRAY || r->type != SYM_ARRAY) {
                printf("Error: array addition only allowed for arrays\n");
                exit(1);
            }
            if (l->size != r->size) {
                printf("Error: array sizes must match for element-wise addition\n");
                exit(1);
            }
        }
    }

    if (n->type == AST_ASSIGN && n->left && n->right) {
        Symbol *lhs = lookup_symbol(n->left->name);
        if (n->right->type == AST_ARRAY_ADD) {
            if (!lhs || lhs->type != SYM_ARRAY) {
                printf("Error: array addition assignment only allowed for arrays\n");
                exit(1);
            }
            // Check size match with operands
            Symbol *l = lookup_symbol(n->right->left->name);
            Symbol *r = lookup_symbol(n->right->right->name);
            if (!l || !r || lhs->size != l->size || lhs->size != r->size) {
                printf("Error: array sizes must match for assignment\n");
                exit(1);
            }
        }
    }

    check_ast(n->left);
    check_ast(n->right);
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

