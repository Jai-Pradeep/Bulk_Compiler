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
  int ival;
}


%token INT FLOAT CHAR
%token <id> ID
%token <id> INT_LITERAL FLOAT_LITERAL CHAR_LITERAL
%token PLUS ASSIGN SEMICOLON LBRACKET RBRACKET COMMA

%type <node> expr stmt
%type <ival> type
%%

program:
      decl_list stmt { root = $2; }
      ;

decl_list:
	 decl_list decl
	| /* empty */
	;


// Support multiple declarations and array sizes
decl:
    type decl_list SEMICOLON
    ;

type:
    INT   { $$ = TYPE_INT; }
  | FLOAT { $$ = TYPE_FLOAT; }
  | CHAR  { $$ = TYPE_CHAR; }
    ;

decl_list:
    decl_item
  | decl_list COMMA decl_item
    ;

decl_item:
    ID
      { insert_symbol($1, SYM_SCALAR, $<ival>-2, 0); }
  | ID LBRACKET INT_LITERAL RBRACKET
      { insert_symbol($1, SYM_ARRAY, $<ival>-4, atoi($3)); }
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
    { $$ = make_id($1); }
  | INT_LITERAL
    { $$ = make_id($1); }
  | FLOAT_LITERAL
    { $$ = make_id($1); }
  | CHAR_LITERAL
    { $$ = make_id($1); }
  | expr PLUS ID
    { $$ = make_add($1, make_id($3)); }
  | expr PLUS INT_LITERAL
    { $$ = make_add($1, make_id($3)); }
  | expr PLUS FLOAT_LITERAL
    { $$ = make_add($1, make_id($3)); }
  | expr PLUS CHAR_LITERAL
    { $$ = make_add($1, make_id($3)); }
  ;

%%

void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}

int main() {
  if (yyparse() == 0) {
    printf("\n=== AST ===\n");
    check_ast(root);
    print_ast(root, 0);
  } else {
    printf("Parsing failed.\n");
  }
  return 0;
}

