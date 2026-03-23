#pragma once
#include <string>
#include <vector>

// ── Types ─────────────────────────────────────────────────────────────────────
enum class IRType { UNKNOWN, INT32, INT64, INT128, VOID };

std::string irTypeName(IRType t);
IRType      parseType (const std::string& s);
IRType      dominantType(IRType a, IRType b);

// ── One three-address instruction ─────────────────────────────────────────────
struct IRInstruction {
    std::string op;      // operator or pseudo-op
    std::string arg1;
    std::string arg2;
    std::string result;
    IRType      type    = IRType::UNKNOWN;
    IRType      srcType = IRType::UNKNOWN;  // used by cast

    IRInstruction(std::string o, std::string a1, std::string a2,
                  std::string r, IRType t = IRType::UNKNOWN);
};

// ── Global IR vector ──────────────────────────────────────────────────────────
extern std::vector<IRInstruction> ir;
extern int tempCount;

// ── Name generators ───────────────────────────────────────────────────────────
std::string newTemp();
std::string newLabel();

// ── Emit helpers ──────────────────────────────────────────────────────────────
void emitLabel    (const std::string& label);
void emitGoto     (const std::string& label);
void emitIfZero   (const std::string& cond, const std::string& lbl);
void emitForLoop  (const std::string& idx, int size);
void emitIf       (const std::string& cond);
void endIf        ();
void endForLoop   ();

// ── Function IR helpers ───────────────────────────────────────────────────────
// func_begin:  marks start of a function definition
//   arg1 = function name,  arg2 = return type string
void emitFuncBegin(const std::string& name, IRType retType);

// func_end:    marks end of a function definition
//   arg1 = function name
void emitFuncEnd  (const std::string& name);

// param:       declare one parameter at function entry
//   arg1 = param name,  type = param type
void emitParam    (const std::string& name, IRType t);

// call:        call a function
//   arg1 = function name,  result = temp that receives return value (or "")
void emitCall     (const std::string& funcName,
                   const std::vector<std::string>& args,
                   const std::string& resultTemp,
                   IRType retType);

// return:      return from a function
//   arg1 = value to return (or "" for void)
void emitReturn   (const std::string& val, IRType t);

// push_arg:    push one argument before a call
//   arg1 = value,  type = its type
void emitPushArg  (const std::string& val, IRType t);

// ── Parallel bulk-array loop ──────────────────────────────────────────────────
void emitParallelLoopHeader(int n,
                            std::string& outIdxVar,
                            std::string& outLstart,
                            std::string& outLend);
void emitParallelLoopFooter(const std::string& idxVar,
                            const std::string& Lstart,
                            const std::string& Lend);

// ── Cast helper ───────────────────────────────────────────────────────────────
std::string emitCastIfNeeded(const std::string& val,
                             IRType fromType, IRType toType);

// ── Output ────────────────────────────────────────────────────────────────────
void printIR();
void writeIRToFile(const std::string& filename);