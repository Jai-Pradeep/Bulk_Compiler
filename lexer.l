%{
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

%%

"int"           { return INT; }
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

";"             { return SEMICOLON; }
","             { return COMMA; }

"("             { return LPAREN; }
")"             { return RPAREN; }

"["             { return LBRACKET; }
"]"             { return RBRACKET; }

[ \t\r\n]+      ;

.               { printf("Unknown character: %s\n", yytext); }

%%

int yywrap() {
    return 1;
}