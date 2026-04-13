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
//     - #pragma omp parallel for  for FULLY_PARALLEL loops (or CUDA kernels)
//     - All arithmetic, comparisons, assignments, if/goto structure
//
//  2. Calls gcc automatically:
//       gcc -O2 -fopenmp output.c -o <exeName>
//       OR if useCUDA: nvcc + gcc with CUDA kernel linking
//
//  Usage:
//    generateCode("output.c", "program", false);  // OpenMP
//    generateCode("output.c", "program", true);   // CUDA
//
void generateCode(const std::string& cFile, const std::string& exeName, bool useCUDA = false);
