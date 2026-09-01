#include <stdio.h>
#include <string>
#include <cstring>
#include "symtab.h"
#include "ir.h"
#include "optimizer.h"
#include "codegen.h"

extern int yyparse();
// Without this line, if you try to call yyparse() inside your main.cpp, the compiler will throw an error like: error: ‘yyparse’ was not declared in this scope.

static void printUsage(const char* prog) {
    printf("\nUsage: %s [options] < input.bc\n\n", prog);
    printf("  -o <name>      output name (default: output)\n");
    printf("                 without --emit-c: writes <name>.ir\n");
    printf("                 with    --emit-c: writes <name>.c and compiles to <name>\n\n");
    printf("  -O0            no optimization (default)\n");
    printf("  -O1            constant folding + copy propagation + dead code\n");
    printf("  -O2            O1 + repeated passes until stable\n\n");
    printf("  --emit-c       generate C + compile to executable via gcc -fopenmp\n");
    printf("  --ir-only      write IR file only, no C (default)\n\n");
    printf("  --cuda         emit CUDA kernel (.cu) for large loops + C fallback\n\n");
    printf("  --emit-asm     generate assembly file (<name>.s)\n");
    printf("  --arch <arch>  target architecture for assembly (default: x86_64)\n");
    printf("                 supported: x86  x86_64  arm  riscv\n\n");
    printf("  --emit-cfg     generate CFG dot file (<name>.dot) + PNG if graphviz available\n\n");
    printf("  --opt-report   write optimization report to <name>.opt.txt\n\n");
    printf("Examples:\n");
    printf("  %s -O2 < prog.bc                              # optimised IR\n", prog);
    printf("  %s -O2 --emit-c -o prog < prog.bc             # parallel executable\n", prog);
    printf("  %s -O2 --emit-c --emit-asm --arch arm < p.bc  # C + ARM assembly\n", prog);
    printf("  %s --emit-cfg -o prog < prog.bc               # CFG dot/png\n", prog);
    printf("  %s -O1 --opt-report -o prog < prog.bc         # optimization report\n\n", prog);
}

int main(int argc, char* argv[]) {
    std::string outName   = "output";
    int         optLevel  = 0;
    bool        emitC     = false;
    bool        useCUDA   = false;
    bool        emitAsm   = false;
    bool        emitCFG   = false;
    bool        optReport = false;
    std::string arch      = "x86_64";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i+1 < argc) {
            outName = argv[++i];
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
            emitC   = true;   // CUDA always requires C generation too
        } else if (strcmp(argv[i], "--emit-asm") == 0) {
            emitAsm = true;
            emitC   = true;   // assembly is generated from the .c file
        } else if (strcmp(argv[i], "--arch") == 0 && i+1 < argc) {
            arch = argv[++i];
            if (arch != "x86" && arch != "x86_64" &&
                arch != "arm" && arch != "riscv") {
                printf("Unknown arch '%s'. Supported: x86  x86_64  arm  riscv\n", arch.c_str());
                return 1;
            }
        } else if (strcmp(argv[i], "--emit-cfg") == 0) {
            emitCFG = true;
        } else if (strcmp(argv[i], "--opt-report") == 0) {
            optReport = true;
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

    std::string irFile      = outName + ".ir";
    std::string cFile       = outName + ".c";
    std::string exeName     = outName;
    std::string asmFile     = outName + ".s";
    std::string dotFile     = outName + ".dot";
    std::string reportFile  = outName + ".opt.txt";

    // ── Parse ────────────────────────────────────────────────────────────────
    printf("=== Parsing ===\n");
    if (yyparse() != 0) {
        printf("Parsing failed!\n");
        return 1;
    }
    printf("=== Parsing complete ===\n\n");

    symtab.print();

    // ── Optimize ─────────────────────────────────────────────────────────────
    size_t irBefore = ir.size();

    if (optLevel > 0) {
        printf("\n=== Optimizer (-O%d) ===\n", optLevel);
        optimizeIR(optLevel);
    } else {
        printf("\n[Optimizer] -O0: no optimization\n");
    }

    size_t irAfter = ir.size();

    // ── IR output ────────────────────────────────────────────────────────────
    printIR();
    writeIRToFile(irFile);

    // ── Optimization report ──────────────────────────────────────────────────
    if (optReport) {
        printf("\n=== Optimization Report ===\n");
        generateOptReport(reportFile, irBefore, irAfter, optLevel);
    }

    // ── CFG ──────────────────────────────────────────────────────────────────
    if (emitCFG) {
        printf("\n=== CFG Generation ===\n");
        generateCFG(dotFile);
    }

    // ── C code generation ────────────────────────────────────────────────────
    if (emitC) {
        printf("\n=== Code Generation ===\n");
        generateCode(cFile, exeName, useCUDA);
    }

    // ── Assembly ─────────────────────────────────────────────────────────────
    // Requires the .c file — generateCode() must have run first.
    if (emitAsm) {
        printf("\n=== Assembly Generation (%s) ===\n", arch.c_str());
        generateAssembly(cFile, asmFile, arch);
    }

    return 0;
}