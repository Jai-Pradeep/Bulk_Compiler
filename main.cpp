#include <stdio.h>
#include <string>
#include <cstring>
#include "symtab.h"
#include "ir.h"
#include "optimizer.h"

extern int yyparse();

static void printUsage(const char* prog) {
    printf("Usage: %s [-o <outfile>] [-O0|-O1|-O2] < input.bc\n", prog);
    printf("  -o <outfile>   write IR to file (default: output.ir)\n");
    printf("  -O0            no optimization (default)\n");
    printf("  -O1            constant folding + copy propagation + dead code elimination\n");
    printf("  -O2            O1 + multiple rounds until fully stable\n");
}

int main(int argc, char* argv[]) {
    std::string irFile   = "output.ir";
    int         optLevel = 0;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i+1 < argc) {
            irFile = argv[++i];
        } else if (strcmp(argv[i], "-O0") == 0) {
            optLevel = 0;
        } else if (strcmp(argv[i], "-O1") == 0) {
            optLevel = 1;
        } else if (strcmp(argv[i], "-O2") == 0) {
            optLevel = 2;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            printUsage(argv[0]);
            return 1;
        }
    }

    // Parse source
    printf("=== Parsing ===\n");
    yyparse();
    printf("=== Parsing complete ===\n\n");

    symtab.print();

    // Optimize based on level
    if (optLevel == 0) {
        printf("\n[Optimizer] -O0: no optimization\n");
    } else {
        printf("\n=== Optimizer (-O%d) ===\n", optLevel);
        optimizeIR(optLevel);
    }

    printIR();
    writeIRToFile(irFile);
    return 0;
}