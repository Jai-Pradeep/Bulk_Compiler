%{
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "ast.h"
#include "symtab.h"
#include "ir.h"

bool emitIR = true;
std::vector<ParamNode>* currentFuncParams = nullptr;
std::string currentFuncName;
std::string currentFuncRetType;

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
%token IF ELSE WHILE FOR SWITCH CASE DEFAULT BREAK CONTINUE
%token FUNC RETURN SCAN PRINT
%token <id>  ID
%token <num> NUMBER
%token PLUS MINUS MUL DIV
%token AND OR NOT
%token BAND BOR BXOR BNOT
%token LSHIFT RSHIFT
%token ASSIGN LT GT LE GE EQ NEQ
%token SEMICOLON COMMA LBRACE RBRACE
%token LPAREN RPAREN LBRACKET RBRACKET

/* Precedence — low to high                          */
/* Logical OR  is lowest                             */
%left OR
%left AND
%left BOR
%left BXOR
%left BAND
%left EQ NEQ
%left LT GT LE GE
%left LSHIFT RSHIFT
%left PLUS MINUS
%left MUL DIV
%right NOT BNOT        /* unary ! and ~ — highest among these   */

%type <node>      expression statement assignment declaration
%type <node>      for_stmt if_stmt while_stmt
%type <node>      func_def func_call_stmt return_stmt
%type <node>      body_stmt body_assignment
%type <node>      scan_stmt print_stmt break_stmt continue_stmt
%type <stmtlist>  body block
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
    | FLOAT   { $$ = (char*)"float";  }   
    | CHAR    { $$ = (char*)"char";   }  
    | BOOL    { $$ = (char*)"bool";   }   
    ;

ret_type_kw:
      INT32    { $$ = (char*)"int32";  }
    | INT64    { $$ = (char*)"int64";  }
    | INT128   { $$ = (char*)"int128"; }
    | VOID_KW  { $$ = (char*)"void";   }
    ;

body:
      /* empty */ { $$ = new StatementListNode(); }
    | body body_stmt {
            $$ = $1;
            $$->add($2);
        }
    ;

block:
      body_stmt { $$ = new StatementListNode(); $$->add($1); }
    | LBRACE body RBRACE { $$ = $2; }
    ;

/* ── body_stmt: statement inside a loop/if — NO generateIR() call ───────── */
/*    The owning node (ForNode, WhileNode, IfNode) calls generateIR() later   */
body_stmt:
      declaration      { $$ = $1; }
    | body_assignment  { $$ = $1; }
    | for_stmt         { $$ = $1; }
    | while_stmt       { $$ = $1; }
    | if_stmt          { $$ = $1; }
    | func_call_stmt   { $$ = $1; }
    | return_stmt      { $$ = $1; }
    | scan_stmt        { $$ = $1; }
    | print_stmt       { $$ = $1; }
    | break_stmt       { $$ = $1; }
    | continue_stmt    { $$ = $1; }
    ;

/* ── body_assignment: builds node only, no IR emission ──────────────────── */
body_assignment:
      expression ASSIGN expression SEMICOLON
        {
            if (auto id = dynamic_cast<IdentifierNode*>($1)) {
                if (!symtab.exists(id->name)) {
                    printf("Error: %s not declared\n", id->name.c_str());
                    exit(1);
                }
                $$ = new AssignmentNode(id->name, $3);
            }
            else if (auto arr = dynamic_cast<ArrayAccessNode*>($1)) {
                if (!symtab.exists(arr->name)) {
                    printf("Error: %s not declared\n", arr->name.c_str());
                    exit(1);
                }
                if (!symtab.get(arr->name).isArray) {
                    printf("Error: %s is not an array\n", arr->name.c_str());
                    exit(1);
                }
                $$ = new ArrayElementAssignmentNode(arr->name, arr->indices, $3);
            }
            else {
                printf("Error: invalid assignment target\n");
                exit(1);
            }
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
    | scan_stmt        { $$ = $1; }
    | print_stmt       { $$ = $1; }
    ;

declaration:
      type_kw ID SEMICOLON
        {
            if (symtab.exists($2)) { printf("Error: redeclaration of %s\n",$2); exit(1); }
            symtab.insert($2, $1, std::vector<int>{});
            $$ = nullptr;
        }
    | type_kw ID LBRACKET NUMBER RBRACKET SEMICOLON
        {
            if (symtab.exists($2)) { printf("Error: redeclaration of %s\n",$2); exit(1); }
            symtab.insert($2, $1, std::vector<int>{static_cast<int>($4)});
            $$ = nullptr;
        }
    | type_kw ID LBRACKET NUMBER RBRACKET LBRACKET NUMBER RBRACKET SEMICOLON
        {
            if (symtab.exists($2)) { printf("Error: redeclaration of %s\n",$2); exit(1); }
            symtab.insert($2, $1, std::vector<int>{static_cast<int>($4), static_cast<int>($7)});
            $$ = nullptr;
        }
    | type_kw ID ASSIGN expression SEMICOLON
        {
            if (symtab.exists($2)) { printf("Error: redeclaration of %s\n",$2); exit(1); }
            symtab.insert($2, $1, std::vector<int>{});
            AssignmentNode* a = new AssignmentNode($2, $4);
            if (emitIR) a->generateIR();
            $$ = a;
        }
    ;

assignment:
      ID ASSIGN expression SEMICOLON
        {
            if (!symtab.exists($1)) { printf("Error: %s not declared\n",$1); exit(1); }
            AssignmentNode* a = new AssignmentNode($1, $3);
            if (emitIR) a->generateIR();
            $$ = a;
        }
    | ID LBRACKET expression RBRACKET ASSIGN expression SEMICOLON
        {
            if (!symtab.exists($1)) { printf("Error: %s not declared\n",$1); exit(1); }
            if (!symtab.get($1).isArray) { printf("Error: %s is not an array\n",$1); exit(1); }
            ArrayElementAssignmentNode* a = new ArrayElementAssignmentNode(std::string($1), {$3}, $6);
            if (emitIR) a->generateIR();
            $$ = a;
        }
    | expression LBRACKET expression RBRACKET ASSIGN expression SEMICOLON
        {
            // Multi-dimensional array assignment: arr[i][j] = val
            ArrayAccessNode* arr = dynamic_cast<ArrayAccessNode*>($1);
            if (!arr) {
                printf("Error: invalid lvalue for assignment\n");
                exit(1);
            }
            // arr->name is the base array, arr->indices has the first index
            std::vector<ASTNode*> allIndices = arr->indices;
            allIndices.push_back($3);
            ArrayElementAssignmentNode* a = new ArrayElementAssignmentNode(arr->name, allIndices, $6);
            delete arr;  // since we took ownership
            if (emitIR) a->generateIR();
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
      FUNC ret_type_kw ID LPAREN param_list RPAREN
        {
            currentFuncRetType = std::string($<id>2);
            currentFuncName    = std::string($<id>3);
            currentFuncParams  = $<paramlist>5;
            // Register function early for recursive calls
            FuncSignature sig;
            sig.returnType = parseType(currentFuncRetType);
            for (auto& p : *currentFuncParams) {
                sig.paramTypes.push_back(parseType(p.typeName));
                sig.paramNames.push_back(p.name);
            }
            symtab.insertFunc(currentFuncName, sig);
            // NOTE: Do NOT enter scope here — let FunctionDefNode::generateIR() handle scope management
            emitIR = false;
        }
      LBRACE body RBRACE
        {
            emitIR = true;
            // NOTE: Do NOT leave scope here — let FunctionDefNode::generateIR() handle it
            FunctionDefNode* f = new FunctionDefNode(
                currentFuncRetType, currentFuncName, *currentFuncParams, $<stmtlist>9);
            delete currentFuncParams;
            currentFuncParams = nullptr;
            // f->print(0);
            if (emitIR) f->generateIR();
            $$ = f;
        }
    ;

func_call_stmt:
      ID LPAREN arg_list RPAREN SEMICOLON
        {
            FunctionCallNode* fc = new FunctionCallNode(std::string($1), *$3);
            delete $3;
            if (emitIR) fc->generateIR();
            $$ = fc;
        }
    ;

return_stmt:
      RETURN expression SEMICOLON
        {
            ReturnNode* r = new ReturnNode($2);
            if (emitIR) r->generateIR();
            $$ = r;
        }
    | RETURN SEMICOLON
        {
            ReturnNode* r = new ReturnNode(nullptr);
            if (emitIR) r->generateIR();
            $$ = r;
        }
    ;

scan_stmt:
      SCAN LPAREN ID RPAREN SEMICOLON
        {
            ScanNode* s = new ScanNode(std::string($3));
            if (emitIR) s->generateIR();
            $$ = s;
        }
    ;

print_stmt:
      PRINT LPAREN expression RPAREN SEMICOLON
        {
            PrintNode* p = new PrintNode($3);
            if (emitIR) p->generateIR();
            $$ = p;
        }
    ;

break_stmt:
      BREAK SEMICOLON
        {
            $$ = new BreakNode();
        }
    ;

continue_stmt:
      CONTINUE SEMICOLON
        {
            $$ = new ContinueNode(); 
        }
    ;

for_stmt:
      FOR LPAREN
          ID ASSIGN expression SEMICOLON
          expression SEMICOLON
          ID ASSIGN expression
      RPAREN block
        {
            if (!symtab.exists($3))     { printf("Error: %s not declared\n", $3);     exit(1); }
            if (!symtab.exists($<id>9)) { printf("Error: %s not declared\n", $<id>9); exit(1); }

            AssignmentNode*    initNode = new AssignmentNode(std::string($3),      $<node>5);
            ASTNode*           condNode = $<node>7;
            AssignmentNode*    stepNode = new AssignmentNode(std::string($<id>9),  $<node>11);
            StatementListNode* bodyNode = $<stmtlist>13;

            ForNode* f = new ForNode(initNode, condNode, stepNode, bodyNode);
            // f->print(0);
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
      WHILE LPAREN expression RPAREN block
        {
            WhileNode* w = new WhileNode($3, $5);
            // w->print(0);
            $$ = w;
        }
    ;

if_stmt:
      IF LPAREN expression RPAREN block
        {
            IfNode* n = new IfNode($3, $5, nullptr);
            // n->print(0);
            $$ = n;
        }
    | IF LPAREN expression RPAREN block ELSE block
        {
            IfNode* n = new IfNode($3, $5, $7);
            // n->print(0);
            $$ = n;
        }
    ;

expression:
      expression PLUS  expression  { $$ = new BinaryOpNode("+",$1,$3); }
    | expression MINUS expression  { $$ = new BinaryOpNode("-",$1,$3); }
    | expression MUL   expression  { $$ = new BinaryOpNode("*",$1,$3); }
    | expression DIV   expression  { $$ = new BinaryOpNode("/",$1,$3); }
    | expression LSHIFT expression { $$ = new BinaryOpNode("<<",$1,$3); }
    | expression RSHIFT expression { $$ = new BinaryOpNode(">>",$1,$3); }
    | expression LT    expression  { $$ = new ComparisonNode("<" ,$1,$3); }
    | expression GT    expression  { $$ = new ComparisonNode(">" ,$1,$3); }
    | expression LE    expression  { $$ = new ComparisonNode("<=",$1,$3); }
    | expression GE    expression  { $$ = new ComparisonNode(">=",$1,$3); }
    | expression EQ    expression  { $$ = new ComparisonNode("==",$1,$3); }
    | expression NEQ   expression  { $$ = new ComparisonNode("!=",$1,$3); }
    | expression BAND  expression  { $$ = new BinaryOpNode("&",$1,$3); }
    | expression BOR   expression  { $$ = new BinaryOpNode("|",$1,$3); }
    | expression BXOR  expression  { $$ = new BinaryOpNode("^",$1,$3); }
    | expression AND   expression  { $$ = new LogicalOpNode("&&",$1,$3); }
    | expression OR    expression  { $$ = new LogicalOpNode("||",$1,$3); }
    | NOT expression               { $$ = new LogicalNotNode($2); }
    | BNOT expression              { $$ = new UnaryOpNode("~",$2); }
    | ID LPAREN arg_list RPAREN
        {
            $$ = new FunctionCallNode(std::string($1), *$3);
            delete $3;
        }
    | ID LBRACKET expression RBRACKET { $$ = new ArrayAccessNode(std::string($1), {$3}); }
    | expression LBRACKET expression RBRACKET
        {
            ArrayAccessNode* arr = dynamic_cast<ArrayAccessNode*>($1);
            if (!arr) {
                printf("Error: invalid array access base\n");
                exit(1);
            }
            arr->indices.push_back($3);
            $$ = arr;
        }
    | ID                              { $$ = new IdentifierNode($1); }
    | NUMBER                          { $$ = new NumberNode($1); }
    ;

%%

void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}