%{
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ast.h"
#include "symtab.h"
#include "ir.h"

void yyerror(const char *s);
int yylex();

ASTNode* root;
%}

%code requires {
#include "ast.h"
#include "ir.h"
}

%union {
    int num;
    char* id;
    ASTNode* node;
}

%token INT FLOAT CHAR BOOL
%token IF ELSE WHILE FOR
%token <id> ID
%token <num> NUMBER

%token PLUS MINUS MUL DIV
%token ASSIGN

%token SEMICOLON COMMA
%token LPAREN RPAREN
%token LBRACKET RBRACKET

%left PLUS MINUS
%left MUL DIV

%type <node> expression assignment

%%

program:
      program statement
    |
    ;

statement:
      declaration
    | assignment { root = $1; }
    ;

declaration:

      INT ID SEMICOLON
        {
            if(symtab.exists($2)) {
                printf("Error: redeclaration of %s\n",$2);
                exit(1);
            }

            symtab.insert($2,"int",false,0);
        }

    | INT ID LBRACKET NUMBER RBRACKET SEMICOLON
        {
            if(symtab.exists($2)) {
                printf("Error: redeclaration of %s\n",$2);
                exit(1);
            }

            symtab.insert($2,"int",true,$4);
        }
;

assignment:

      ID ASSIGN expression SEMICOLON
        {
            if(!symtab.exists($1)) {
                printf("Error: variable %s not declared\n",$1);
                exit(1);
            }

            $$ = new AssignmentNode($1,$3);

            $$->print(0);

            $$->generateIR();

            printIR();
        }
;

expression:

      expression PLUS expression
        { $$ = new BinaryOpNode("+",$1,$3); }

    | expression MINUS expression
        { $$ = new BinaryOpNode("-",$1,$3); }

    | expression MUL expression
        { $$ = new BinaryOpNode("*",$1,$3); }

    | expression DIV expression
        { $$ = new BinaryOpNode("/",$1,$3); }

    | ID LBRACKET expression RBRACKET
        { $$ = new ArrayAccessNode($1,$3); }

    | ID
        { $$ = new IdentifierNode($1); }

    | NUMBER
        { $$ = new NumberNode($1); }
;

%%

void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}