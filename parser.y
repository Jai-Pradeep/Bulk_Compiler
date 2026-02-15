%code requires {
    typedef struct ASTNode ASTNode;
}


%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

void yyerror(const char *s);
int yylex();

ASTNode *root;
%}

%union {
    char *id;
    ASTNode *node;
}

%token INT
%token <id> ID
%token PLUS ASSIGN SEMICOLON LBRACKET RBRACKET

%type <node> expr stmt

%%

program:
      decl_list stmt { root = $2; }
      ;

decl_list:
	 decl_list decl
	| /* empty */
	;

decl:
    INT ID SEMICOLON
	{ insert_symbol($2, SYM_SCALAR);}
	| INT ID LBRACKET RBRACKET SEMICOLON
	{ insert_symbol($2, SYM_ARRAY);}
	;

stmt:
      ID ASSIGN expr SEMICOLON
        {
          Symbol *s = lookup_symbol($1);
          if (!s) {
              printf("Error: undeclared variable %s\n", $1);
              exit(1);
          }
          $$ = make_assign(make_id($1), $3);
        }
      ;

expr:
      ID
        {
          $$ = make_id($1);
        }
    | expr PLUS ID
        {
          $$ = make_add($1, make_id($3));
        }
    ;

%%

void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}

int main() {
    yyparse();
    printf("\n=== AST ===\n");
    check_ast(root);
    print_ast(root, 0);
    return 0;
}

