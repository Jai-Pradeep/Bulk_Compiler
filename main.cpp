#include <stdio.h>
#include <string>
#include <cstring>
#include "symtab.h"
#include "ir.h"
#include "optimizer.h"
#include "codegen.h"

extern int yyparse();

static void printUsage(const char* prog) {
    printf("\nUsage: %s [options] < input.bc\n\n", prog);
    printf("  -o <name>      output name (default: program)\n");
    printf("                 without --emit-c: writes <name>.ir\n");
    printf("                 with    --emit-c: writes <name>.c and compiles to <name>\n\n");
    printf("  -O0            no optimization (default)\n");
    printf("  -O1            constant folding + copy propagation + dead code\n");
    printf("  -O2            O1 + repeated passes until stable\n\n");
    printf("  --emit-c       generate C + compile to executable via gcc -fopenmp\n");
    printf("  --ir-only      write IR file only, no C (default)\n\n");
    printf("Examples:\n");
    printf("  %s -O2 < prog.bc                   # optimised IR to output.ir\n", prog);
    printf("  %s -O2 --emit-c -o prog < prog.bc  # parallel executable ./prog\n", prog);
    printf("  %s -O1 -o myout < prog.bc           # IR to myout.ir\n\n", prog);
}

int main(int argc, char* argv[]) {
    std::string outName  = "output";   // base name — .ir or executable
    int         optLevel = 0;
    bool        emitC    = false;
    bool        useCUDA  = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i+1 < argc) {
            outName = argv[++i];
            // strip any extension the user typed
            auto dot = outName.rfind('.');
            if (dot != std::string::npos)
                outName = outName.substr(0, dot);
        } else if (strcmp(argv[i], "-O0") == 0) {
            optLevel = 0;
        } else if (strcmp(argv[i], "-O1") == 0) {
            optLevel = 1;
        } else if (strcmp(argv[i], "-O2") == 0) {
            optLevel = 2;
        } else if (strcmp(argv[i], "--emit-c") == 0) {
            emitC = true;
        } else if (strcmp(argv[i], "--ir-only") == 0) {
            emitC = false;
        } else if (strcmp(argv[i], "--cuda") == 0) {
            useCUDA = true;
            emitC = true;  // CUDA requires C generation
        } else if (strcmp(argv[i], "-h") == 0 ||
                   strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            printUsage(argv[0]);
            return 1;
        }
    }

    std::string irFile  = outName + ".ir";
    std::string cFile   = outName + ".c";
    std::string exeName = outName;

    // Parse
    printf("=== Parsing ===\n");
    yyparse();
    printf("=== Parsing complete ===\n\n");

    symtab.print();

    // Optimize
    if (optLevel > 0) {
        printf("\n=== Optimizer (-O%d) ===\n", optLevel);
        optimizeIR(optLevel);
    } else {
        printf("\n[Optimizer] -O0: no optimization\n");
    }

    // Always write IR
    printIR();
    writeIRToFile(irFile);

    // Optionally generate C and compile
    if (emitC) {
        printf("\n=== Code Generation ===\n");
        generateCode(cFile, exeName, useCUDA);
    }

    return 0;
}