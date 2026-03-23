%{
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "ast.h"
#include "symtab.h"
#include "ir.h"

void yyerror(const char *s);
int yylex();
%}

%code requires {
#include "ast.h"
#include "ir.h"
#include <vector>
}

%union {
    int num;
    char* id;
    ASTNode*                  node;
    StatementListNode*        stmtlist;
    std::vector<ParamNode>*   paramlist;
    std::vector<ASTNode*>*    arglist;
}

%token INT32 INT64 INT128
%token FLOAT CHAR BOOL VOID_KW
%token IF ELSE WHILE FOR
%token FUNC RETURN
%token <id>  ID
%token <num> NUMBER
%token PLUS MINUS MUL DIV
%token AND OR NOT
%token ASSIGN LT GT LE GE EQ NEQ
%token SEMICOLON COMMA LBRACE RBRACE
%token LPAREN RPAREN LBRACKET RBRACKET

/* Precedence — low to high                          */
/* Logical OR  is lowest                             */
%left OR
%left AND
%left EQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV
%right NOT        /* unary ! — highest among these   */

%type <node>      expression statement assignment declaration
%type <node>      for_stmt if_stmt while_stmt
%type <node>      func_def func_call_stmt return_stmt
%type <stmtlist>  body
%type <id>        type_kw ret_type_kw
%type <paramlist> param_list param_list_ne
%type <arglist>   arg_list arg_list_ne

%%

program:
      program statement
    | statement
    ;

type_kw:
      INT32   { $$ = (char*)"int32";  }
    | INT64   { $$ = (char*)"int64";  }
    | INT128  { $$ = (char*)"int128"; }
    ;

ret_type_kw:
      INT32    { $$ = (char*)"int32";  }
    | INT64    { $$ = (char*)"int64";  }
    | INT128   { $$ = (char*)"int128"; }
    | VOID_KW  { $$ = (char*)"void";   }
    ;

body:
      /* empty */ { $$ = new StatementListNode(); }
    | body statement {
            $$ = $1;
            $$->add($2);
        }
    ;

statement:
      declaration      { $$ = $1; }
    | assignment       { $$ = $1; }
    | for_stmt         { $$ = $1; }
    | while_stmt       { $$ = $1; }
    | if_stmt          { $$ = $1; }
    | func_def         { $$ = $1; }
    | func_call_stmt   { $$ = $1; }
    | return_stmt      { $$ = $1; }
    ;

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

param_list:
      /* empty */   { $$ = new std::vector<ParamNode>(); }
    | param_list_ne { $$ = $1; }
    ;

param_list_ne:
      type_kw ID
        {
            $$ = new std::vector<ParamNode>();
            $$->push_back({std::string($1), std::string($2)});
        }
    | param_list_ne COMMA type_kw ID
        {
            $$ = $1;
            $$->push_back({std::string($3), std::string($4)});
        }
    ;

arg_list:
      /* empty */  { $$ = new std::vector<ASTNode*>(); }
    | arg_list_ne  { $$ = $1; }
    ;

arg_list_ne:
      expression
        {
            $$ = new std::vector<ASTNode*>();
            $$->push_back($1);
        }
    | arg_list_ne COMMA expression
        {
            $$ = $1;
            $$->push_back($3);
        }
    ;

func_def:
      FUNC ret_type_kw ID LPAREN param_list RPAREN LBRACE body RBRACE
        {
            FunctionDefNode* f = new FunctionDefNode(
                std::string($2), std::string($3), *$5, $8);
            delete $5;
            f->print(0);
            f->generateIR();
            $$ = f;
        }
    ;

func_call_stmt:
      ID LPAREN arg_list RPAREN SEMICOLON
        {
            FunctionCallNode* fc = new FunctionCallNode(std::string($1), *$3);
            delete $3;
            fc->generateIR();
            $$ = fc;
        }
    ;

return_stmt:
      RETURN expression SEMICOLON
        {
            ReturnNode* r = new ReturnNode($2);
            r->generateIR();
            $$ = r;
        }
    | RETURN SEMICOLON
        {
            ReturnNode* r = new ReturnNode(nullptr);
            r->generateIR();
            $$ = r;
        }
    ;

for_stmt:
      FOR LPAREN
          ID ASSIGN expression SEMICOLON
          expression SEMICOLON
          ID ASSIGN expression
      RPAREN LBRACE body RBRACE
        {
            if (!symtab.exists($3))     { printf("Error: %s not declared\n", $3);     exit(1); }
            if (!symtab.exists($<id>9)) { printf("Error: %s not declared\n", $<id>9); exit(1); }

            AssignmentNode*    initNode = new AssignmentNode(std::string($3),      $<node>5);
            ASTNode*           condNode = $<node>7;
            AssignmentNode*    stepNode = new AssignmentNode(std::string($<id>9),  $<node>11);
            StatementListNode* bodyNode = $<stmtlist>14;

            ForNode* f = new ForNode(initNode, condNode, stepNode, bodyNode);
            f->print(0);
            f->generateIR();
            $$ = f;
        }
    ;

/* ── while loop ──────────────────────────────────────────────────────────── */
/*   while (cond) { body }                                                    */
/*                                                                            */
/*   IR layout:                                                               */
/*   Lstart:                                                                  */
/*     <cond>                                                                 */
/*     if cond == 0 goto Lend                                                 */
/*     <body>                                                                 */
/*     goto Lstart                                                            */
/*   Lend:                                                                    */

while_stmt:
      WHILE LPAREN expression RPAREN LBRACE body RBRACE
        {
            WhileNode* w = new WhileNode($3, $6);
            w->print(0);
            w->generateIR();
            $$ = w;
        }
    ;

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
    | expression NEQ   expression  { $$ = new ComparisonNode("!=",$1,$3); }
    | expression AND   expression  { $$ = new LogicalOpNode("&&",$1,$3); }
    | expression OR    expression  { $$ = new LogicalOpNode("||",$1,$3); }
    | NOT expression               { $$ = new LogicalNotNode($2); }
    | ID LPAREN arg_list RPAREN
        {
            $$ = new FunctionCallNode(std::string($1), *$3);
            delete $3;
        }
    | ID LBRACKET expression RBRACKET { $$ = new ArrayAccessNode($1,$3); }
    | ID                              { $$ = new IdentifierNode($1); }
    | NUMBER                          { $$ = new NumberNode($1); }
    ;

%%

void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}
