#pragma once
#include <string>
#include <vector>
#include <fstream>

// ── Supported scalar types ────────────────────────────────────────────────────
enum class IRType {
    INT32,
    INT64,
    INT128,
    UNKNOWN   // temporaries whose type is not yet determined
};

std::string irTypeName(IRType t);          // "i32" / "i64" / "i128"
IRType      parseType(const std::string& s); // "int32"/"int64"/"int128" -> IRType
IRType      dominantType(IRType a, IRType b);// widening promotion

// ── One three-address IR instruction ─────────────────────────────────────────
struct IRInstruction {
    std::string op;      // "+","-","*","/","<",">","<=",">=","==",
                         // "=","label","goto","ifzero_goto","cast","comment"
    std::string arg1;
    std::string arg2;
    std::string result;

    IRType type    = IRType::UNKNOWN;  // result type annotation
    IRType srcType = IRType::UNKNOWN;  // used by "cast" only

    IRInstruction(std::string o, std::string a1, std::string a2, std::string r,
                  IRType t = IRType::UNKNOWN);
};

// ── Global IR buffer ──────────────────────────────────────────────────────────
extern std::vector<IRInstruction> ir;
extern int tempCount;

// ── Helpers emitted by AST nodes ─────────────────────────────────────────────
std::string newTemp();
std::string newLabel();

void emitLabel   (const std::string& label);
void emitGoto    (const std::string& label);
void emitIfZero  (const std::string& cond, const std::string& label);
void emitForLoop (const std::string& idx, int size);  // legacy array-loop comment
void emitIf      (const std::string& cond);
void endIf();
void endForLoop();

//   // PARALLEL LOOP — no loop-carried dependencies, safe for SIMD / multi-thread
//   i = 0
// Lstart:
//   if i >= n goto Lend
//   <body instructions inserted by caller>
//   i = i + 1
//   goto Lstart
// Lend:
//
// Usage pattern:
//   std::string iVar, Lend;
//   emitParallelLoopHeader(n, iVar, Lend);   // emits init + label + bound check
//   ... emit body instructions using iVar ...
//   emitParallelLoopFooter(iVar, Lstart, Lend); // emits step + goto + end label
//
// Simpler all-in-one version (body instructions must be added before calling footer):
void emitParallelLoopHeader(int n, std::string& outIdxVar, std::string& outLstart, std::string& outLend);
void emitParallelLoopFooter(const std::string& idxVar, const std::string& Lstart, const std::string& Lend);

// Emit a widening cast if fromType != toType; returns dest temp (or val unchanged)
std::string emitCastIfNeeded(const std::string& val, IRType fromType, IRType toType);

// ── Output ────────────────────────────────────────────────────────────────────
void printIR();
void writeIRToFile(const std::string& filename);