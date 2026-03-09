#include <stdio.h>
#include <string>
#include "symtab.h"
#include "ir.h"

extern int yyparse();

int main(int argc, char* argv[]) {
    // Determine output IR filename
    std::string irFile = "output.ir";
    if (argc >= 3 && std::string(argv[1]) == "-o")
        irFile = argv[2];

    printf("=== Parsing ===\n");
    yyparse();
    printf("=== Parsing complete ===\n\n");

    symtab.print();       // symbol table to stdout

    printIR();            // IR summary to stdout (for quick debug)

    writeIRToFile(irFile); // clean IR written to file
    return 0;
}