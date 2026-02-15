%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"

int yywrap(void) { return 1; }
%}

%%
"int" 		{ return INT; }
"["   		{ return LBRACKET; }
"]"   		{ return RBRACKET; }
"+"             { return PLUS; }
"="             { return ASSIGN; }
";"             { return SEMICOLON; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.id = strdup(yytext);
    return ID;
}

[ \t\n]+        ;
.               ;
%%

