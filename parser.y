%{
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ast.h"
#include "symtab.h"
#include "ir.h"

void yyerror(const char *s);
int yylex();
%}

%code requires {
#include "ast.h"
#include "ir.h"
}

%union {
    int num;
    char* id;
    ASTNode*           node;
    StatementListNode* stmtlist;
}

%token INT32 INT64 INT128
%token FLOAT CHAR BOOL
%token IF ELSE WHILE FOR
%token <id>  ID
%token <num> NUMBER
%token PLUS MINUS MUL DIV
%token ASSIGN LT GT LE GE EQ
%token SEMICOLON COMMA LBRACE RBRACE
%token LPAREN RPAREN LBRACKET RBRACKET

%left PLUS MINUS
%left MUL DIV
%left LT GT LE GE EQ

%type <node>     expression statement assignment declaration for_stmt if_stmt
%type <stmtlist> body
%type <id>       type_kw

%%

program:
      program statement
    | statement
    ;

/* ── type keyword ────────────────────────────────────────────────────────── */
type_kw:
      INT32   { $$ = (char*)"int32";  }
    | INT64   { $$ = (char*)"int64";  }
    | INT128  { $$ = (char*)"int128"; }
    ;

/* ── body: list of statements collected into a StatementListNode ─────────── */
/*    IMPORTANT: statements inside a body are NOT auto-emitted.               */
/*    The owning ForNode / IfNode calls generateIR() at the right time.       */
body:
      /* empty */ { $$ = new StatementListNode(); }
    | body statement {
            $$ = $1;
            $$->add($2);
        }
    ;

/* ── top-level statement (emits IR immediately) ─────────────────────────── */
statement:
      declaration { $$ = $1; }
    | assignment  { $$ = $1; }
    | for_stmt    { $$ = $1; }
    | if_stmt     { $$ = $1; }
    ;

/* ── declarations ────────────────────────────────────────────────────────── */
declaration:
      type_kw ID SEMICOLON
        {
            if (symtab.exists($2)) { printf("Error: redeclaration of %s\n",$2); exit(1); }
            symtab.insert($2, $1, false, 0);
            $$ = nullptr;
        }
    | type_kw ID LBRACKET NUMBER RBRACKET SEMICOLON
        {
            if (symtab.exists($2)) { printf("Error: redeclaration of %s\n",$2); exit(1); }
            symtab.insert($2, $1, true, $4);
            $$ = nullptr;
        }
    | type_kw ID ASSIGN expression SEMICOLON
        {
            if (symtab.exists($2)) { printf("Error: redeclaration of %s\n",$2); exit(1); }
            symtab.insert($2, $1, false, 0);
            AssignmentNode* a = new AssignmentNode($2, $4);
            a->generateIR();
            $$ = a;
        }
    ;

/* ── assignments (top-level: emit immediately) ───────────────────────────── */
assignment:
      ID ASSIGN expression SEMICOLON
        {
            if (!symtab.exists($1)) { printf("Error: %s not declared\n",$1); exit(1); }
            AssignmentNode* a = new AssignmentNode($1, $3);
            a->generateIR();
            $$ = a;
        }
    | ID LBRACKET expression RBRACKET ASSIGN expression SEMICOLON
        {
            if (!symtab.exists($1)) { printf("Error: %s not declared\n",$1); exit(1); }
            if (!symtab.get($1).isArray) { printf("Error: %s is not an array\n",$1); exit(1); }
            ArrayElementAssignmentNode* a = new ArrayElementAssignmentNode($1, $3, $6);
            a->generateIR();
            $$ = a;
        }
    ;

/* ── for loop ────────────────────────────────────────────────────────────── */
/*
 *  for ( ID = expr ; expr < expr ; ID = expr ) { body }
 *
 *  Build AST nodes for init / cond / step WITHOUT calling generateIR().
 *  Pass them to ForNode together with the body StatementListNode.
 *  ForNode::generateIR() emits everything in the correct order.
 */

 for_stmt:
      FOR LPAREN
          ID ASSIGN expression SEMICOLON
          expression SEMICOLON
          ID ASSIGN expression
      RPAREN LBRACE body RBRACE
        {
            /* positions:
               1=FOR 2=LPAREN 3=ID 4=ASSIGN 5=expr 6=SEMI
               7=expr(full condition) 8=SEMI
               9=ID 10=ASSIGN 11=expr
               12=RPAREN 13=LBRACE 14=body 15=RBRACE       */

            if (!symtab.exists($3))      { printf("Error: %s not declared\n", $3);      exit(1); }
            if (!symtab.exists($<id>9))  { printf("Error: %s not declared\n", $<id>9);  exit(1); }

            AssignmentNode* initNode = new AssignmentNode(std::string($3),      $<node>5);
            ASTNode*        condNode = $<node>7;
            AssignmentNode* stepNode = new AssignmentNode(std::string($<id>9),  $<node>11);
            StatementListNode* bodyNode = $<stmtlist>14;

            ForNode* f = new ForNode(initNode, condNode, stepNode, bodyNode);
            f->print(0);
            f->generateIR();
            $$ = f;
        }
    ;

/* ── if / if-else ────────────────────────────────────────────────────────── */
if_stmt:
      IF LPAREN expression RPAREN LBRACE body RBRACE
        {
            IfNode* n = new IfNode($3, $6, nullptr);
            n->print(0);
            n->generateIR();
            $$ = n;
        }
    | IF LPAREN expression RPAREN LBRACE body RBRACE ELSE LBRACE body RBRACE
        {
            IfNode* n = new IfNode($3, $6, $10);
            n->print(0);
            n->generateIR();
            $$ = n;
        }
    ;

/* ── expressions ─────────────────────────────────────────────────────────── */
expression:
      expression PLUS  expression  { $$ = new BinaryOpNode("+",$1,$3); }
    | expression MINUS expression  { $$ = new BinaryOpNode("-",$1,$3); }
    | expression MUL   expression  { $$ = new BinaryOpNode("*",$1,$3); }
    | expression DIV   expression  { $$ = new BinaryOpNode("/",$1,$3); }
    | expression LT    expression  { $$ = new ComparisonNode("<" ,$1,$3); }
    | expression GT    expression  { $$ = new ComparisonNode(">" ,$1,$3); }
    | expression LE    expression  { $$ = new ComparisonNode("<=",$1,$3); }
    | expression GE    expression  { $$ = new ComparisonNode(">=",$1,$3); }
    | expression EQ    expression  { $$ = new ComparisonNode("==",$1,$3); }
    | ID LBRACKET expression RBRACKET { $$ = new ArrayAccessNode($1,$3); }
    | ID                              { $$ = new IdentifierNode($1); }
    | NUMBER                          { $$ = new NumberNode($1); }
    ;

%%

void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}
