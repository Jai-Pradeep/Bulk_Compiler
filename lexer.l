%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"

int yywrap(void) { return 1; }
%}

%%
"int"           { return INT; }
"float"         { return FLOAT; }
"char"          { return CHAR; }
","             { return COMMA; }
"["             { return LBRACKET; }

[0-9]+           { yylval.id = strdup(yytext); return INT_LITERAL; }
"]"             { return RBRACKET; }
"+"             { return PLUS; }
"="             { return ASSIGN; }
";"             { return SEMICOLON; }

[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|\.[0-9]+([eE][+-]?[0-9]+)? {
    yylval.id = strdup(yytext);
    return FLOAT_LITERAL;
}

[0-9]+ {
    yylval.id = strdup(yytext);
    return INT_LITERAL;
}

'([^'\\]|\\.)' {
    yylval.id = strdup(yytext);
    return CHAR_LITERAL;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.id = strdup(yytext);
    return ID;
}

[ \t\n]+        ;

. {
    printf("Lexer error: invalid character '%s'\n", yytext);
    exit(1);
}
%%

