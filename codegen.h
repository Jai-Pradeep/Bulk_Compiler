#pragma once
#include <string>
#include "symtab.h"

// ── Code Generator ────────────────────────────────────────────────────────────
//
//  Reads the global `ir` vector and the symbol table, then:
//
//  1. Writes a complete, compilable C file  (output.c)
//     - Global variable declarations from the symbol table
//     - Function definitions from func_begin/func_end blocks
//     - Sequential loops for NOT_PARALLEL / UNKNOWN loops
//     - #pragma omp parallel for  for FULLY_PARALLEL loops
//     - All arithmetic, comparisons, assignments, if/goto structure
//
//  2. Calls gcc automatically:
//       gcc -O2 -fopenmp output.c -o <exeName>
//
//  Usage:
//    generateCode("output.c", "program");
//
void generateCode(const std::string& cFile, const std::string& exeName);
