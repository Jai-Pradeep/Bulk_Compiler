%code requires {
    typedef struct ASTNode ASTNode;
}


%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

void yyerror(const char *s);
int yylex();

ASTNode *root;
%}

/* IMPORTANT: ast.h must be known BEFORE union */
%union {
    char *id;
    ASTNode *node;
}

%token <id> ID
%token PLUS ASSIGN SEMICOLON

%type <node> expr stmt

%%

program:
      stmt { root = $1; }
      ;

stmt:
      ID ASSIGN expr SEMICOLON
        { $$ = make_assign(make_id($1), $3); }
      ;

expr:
      ID
        { $$ = make_id($1); }
    | expr PLUS ID
        { $$ = make_add($1, make_id($3)); }
    ;

%%

void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}

int main() {
    yyparse();
    printf("\n=== AST ===\n");
    print_ast(root, 0);
    return 0;
}

