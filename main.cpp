#include <stdio.h>

extern int yyparse();

int main() {

    printf("Starting parser...\n");

    yyparse();

    printf("Parsing finished.\n");

    return 0;
}