#include <stdio.h>
#include <string>
#include "symtab.h"
#include "ir.h"
#include "regalloc.h"
#include "codegen.h"  // NEW

extern int yyparse();

int main(int argc, char* argv[]) {
    std::string irFile = "output.ir";
    std::string asmFile = "output.asm";  // NEW
    
    if (argc >= 3 && std::string(argv[1]) == "-o") {
        asmFile = argv[2];  // -o specifies assembly output
        // IR goes to default output.ir
    }

    printf("=== Parsing ===\n");
    yyparse();
    printf("=== Parsing complete ===\n\n");

    symtab.print();
    printIR();

    // NEW: Register allocation
    printf("=== Register Allocation ===\n");
    RegisterAllocation alloc = allocateRegisters(ir);
    printf("Allocated registers and %d bytes stack\n", alloc.stackSize);

    // NEW: Generate assembly
    printf("=== Code Generation ===\n");
    generateAssembly(asmFile, alloc);  // NEW function

    writeIRToFile(irFile);
    return 0;
}