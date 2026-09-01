
<div align="center">

# Bulk Compiler

A custom compiler for a bulk-processing language supporting arrays, parallel loops, and multiple backends (IR, C, CUDA). This project demonstrates a full compiler pipeline: parsing, semantic analysis, IR generation, optimization, code generation, and backend integration.
</div>

---


## Requirements

- **C++17** compiler (e.g., `g++`)
- **Flex** (lexical analyzer generator)
- **Bison** (parser generator)
- **GCC** (for C backend, with OpenMP support)
- **CUDA Toolkit** (for CUDA backend, optional)
- **Graphviz** (for CFG PNG output, optional)

### Ubuntu/Debian Install:
```bash
sudo apt update
sudo apt install build-essential flex bison gcc g++ graphviz
# For CUDA backend (optional):
sudo apt install nvidia-cuda-toolkit
```

---

## Build Instructions

1. **Clone the repository** (if not already):
   ```bash
   git clone https://github.com/Jai-Pradeep/Bulk_Compiler.git
   cd Bulk_Compiler
   git checkout v2
   ```
2. **Build the compiler:**
   ```bash
   make
   ```
   This produces the `compiler` executable.

---

## Usage

```bash
./compiler [options] < input.bc
```

---

### Options

* `-o <name>`
  Output base name (default: `output`)

  * Without `--emit-c`: generates `<name>.ir`
  * With `--emit-c`: generates `<name>.c` and executable `<name>_cpu`

* Optimization Levels:

  * `-O0` → No optimization (default)
  * `-O1` → Constant folding + copy propagation + dead code elimination
  * `-O2` → O1 + repeated passes until convergence

* Code Generation:

  * `--emit-c` → Generate C code and compile (uses `gcc -fopenmp`)
  * `--ir-only` → Only generate IR (default behavior)

* CUDA Support:

  * `--cuda` → Generate CUDA kernel (`.cu`) for large loops (also enables C generation)

* CFG:

  * `--emit-cfg` → Generate CFG `.dot` file (+ PNG if Graphviz installed)

* Reports:

  * `--opt-report` → Generate optimization report (`<name>.opt.txt`)

* Help:

  * `-h`, `--help` → Show usage
* Compilation
    
  * `./<name>_cpu` for executing c file
  * `./<name>_gpu` for executing cu file
  * `time ./<name>_cpu` or `time ./<name>_gpu` for time analysis
  * File ending with `UO` are unoptimised code, written to  check the performance 
   - example : `test_CUDA_UO.c(u)`

  ---

### Examples

```bash
# Generate optimized IR
./compiler -O2 < prog.bc

# Generate parallel executable
./compiler -O2 --emit-c -o prog < prog.bc

# Generate CFG
./compiler --emit-cfg -o prog < prog.bc

# Optimization report
./compiler -O1 --opt-report -o prog < prog.bc

# All together
./compiler -O2 --emit-c --cuda --emit-cfg --opt-report -o myprog < prog.bc
```


## Features

- **Custom Language**: Supports `int32`, `float`, `char`, `bool`, arrays, functions, control flow (`if`, `for`, `while`), and I/O (`scan`, `print`).
- **Intermediate Representation (IR)**: Three-address code with explicit temporaries and labels.
- **Optimizations**: Constant folding, copy propagation, dead code elimination, common subexpression elimination, loop-invariant code motion.
- **Parallelism**: Detects parallelizable loops and emits OpenMP or CUDA code.
- **Multiple Backends**:
  - IR output (.ir)
  - C code generation (.c)
  - Executable via GCC (`-fopenmp`)
  - CUDA kernel emission (.cu)
  - Assembly output (`.s`)
  - Control Flow Graph (CFG) generation (.dot/PNG)
- **Extensive Error Checking**: Redeclaration, undeclared variables, type mismatches, etc.
- **Test Suite**: See Test for sample programs and error cases.

---

## Project Structure

- main.cpp         — Entry point, argument parsing, pipeline control
- lexer.l          — Flex lexer (tokenizes input)
- parser.y         — Bison parser (generates AST)
- `ast.h/cpp`        — Abstract Syntax Tree nodes and IR generation
- `symtab.h/cpp`     — Symbol table (scoped variables, functions)
- `ir.h/cpp`         — IR instruction definitions and helpers
- `optimizer.h/cpp`  — IR optimizations
- `depcheck.h/cpp`   — Loop dependency analysis for parallelism
- `codegen.h/cpp`    — C/CUDA/Assembly code generation, backend integration
- Test            — Test cases (valid and error programs)
- Makefile         — Build rules (including Flex/Bison integration)

---

## License

This project is for educational and research use. Attribution required for reuse.

