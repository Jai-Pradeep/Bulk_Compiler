%{
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

%%

"int128"        { return INT128; }
"int64"         { return INT64; }
"int32"         { return INT32; }
"int"           { return INT32; }    /* legacy: "int" maps to int32 */
"float"         { return FLOAT; }
"char"          { return CHAR; }
"bool"          { return BOOL; }

"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }

[0-9]+          { yylval.num = atoi(yytext); return NUMBER; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.id = strdup(yytext);
    return ID;
}

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MUL; }
"/"             { return DIV; }

"="             { return ASSIGN; }
"<"             { return LT; }
">"             { return GT; }
"<="            { return LE; }
">="            { return GE; }
"=="            { return EQ; }

";"             { return SEMICOLON; }
","             { return COMMA; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }

"("             { return LPAREN; }
")"             { return RPAREN; }

"["             { return LBRACKET; }
"]"             { return RBRACKET; }

"//".*          ;   /* single-line comments */

[ \t\r\n]+      ;

.               { printf("Unknown character: %s\n", yytext); }

%%

int yywrap() { return 1; }
