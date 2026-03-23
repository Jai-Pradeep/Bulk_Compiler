#include "ir.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

// ── Globals ───────────────────────────────────────────────────────────────────
std::vector<IRInstruction> ir;
int tempCount = 0;

// ── Type helpers ──────────────────────────────────────────────────────────────
std::string irTypeName(IRType t) {
    switch (t) {
        case IRType::INT32:  return "i32";
        case IRType::INT64:  return "i64";
        case IRType::INT128: return "i128";
        default:             return "?";
    }
}

IRType parseType(const std::string& s) {
    if (s == "int32" || s == "int") return IRType::INT32;
    if (s == "int64")               return IRType::INT64;
    if (s == "int128")              return IRType::INT128;
    throw std::runtime_error("Unknown type keyword: " + s);
}

// Widening: always promote to the wider type
IRType dominantType(IRType a, IRType b) {
    if (a == IRType::INT128 || b == IRType::INT128) return IRType::INT128;
    if (a == IRType::INT64  || b == IRType::INT64)  return IRType::INT64;
    return IRType::INT32;
}

// ── IRInstruction ctor ────────────────────────────────────────────────────────
IRInstruction::IRInstruction(std::string o, std::string a1, std::string a2,
                             std::string r, IRType t)
    : op(o), arg1(a1), arg2(a2), result(r), type(t) {}

// ── Name generators ───────────────────────────────────────────────────────────
std::string newTemp()  { return "t" + std::to_string(++tempCount); }
std::string newLabel() { return "L" + std::to_string(++tempCount); }

// ── Emit helpers (everything goes into the `ir` vector) ──────────────────────
void emitLabel  (const std::string& label)                      { ir.emplace_back("label",        label, "",    ""); }
void emitGoto   (const std::string& label)                      { ir.emplace_back("goto",         label, "",    ""); }
void emitIfZero (const std::string& cond, const std::string& lbl){ ir.emplace_back("ifzero_goto",  cond,  lbl,  ""); }
void emitForLoop(const std::string& idx, int size)              { ir.emplace_back("comment", "FOR " + idx + " = 0 .. " + std::to_string(size-1), "", ""); }
void emitIf     (const std::string& cond)                       { ir.emplace_back("comment", "IF "      + cond, "", ""); }
void endIf      ()                                              { ir.emplace_back("comment", "END IF",           "", ""); }
void endForLoop ()                                              { ir.emplace_back("comment", "END FOR",          "", ""); }

// ── Parallel bulk-array loop ──────────────────────────────────────────────────
//
// Emits a counted loop over [0, n) with a PARALLEL annotation.
// Each iteration is independent — no loop-carried dependencies —
// so any backend can safely vectorise (SIMD) or distribute across threads.
//
// IR layout:
//   // PARALLEL LOOP [0..n-1] — no loop-carried dependencies, SIMD/thread-safe
//   i_Lx = 0                       [i32]
// Lx:
//   tBound = i_Lx < n              [i32]
//   if tBound == 0 goto Ly
//   < body inserted by caller >
//   tStep  = i_Lx + 1              [i32]
//   i_Lx   = tStep                 [i32]
//   goto Lx
// Ly:
//
void emitParallelLoopHeader(int n, std::string& outIdxVar,
                            std::string& outLstart, std::string& outLend)
{
    outLstart = newLabel();
    outLend   = newLabel();
    outIdxVar = "i_" + outLstart;   // unique per loop — no shadowing in nested loops

    ir.emplace_back("comment",
        "PARALLEL LOOP [0.." + std::to_string(n-1) +
        "]  -- no loop-carried dependencies, SIMD/thread-safe", "", "");

    // Init
    ir.emplace_back("=", "0", "", outIdxVar, IRType::INT32);

    // Header label
    emitLabel(outLstart);

    // Bound check
    std::string tBound = newTemp();
    ir.emplace_back("<", outIdxVar, std::to_string(n), tBound, IRType::INT32);
    emitIfZero(tBound, outLend);
}

void emitParallelLoopFooter(const std::string& idxVar,
                            const std::string& Lstart, const std::string& Lend)
{
    // Step
    std::string tStep = newTemp();
    ir.emplace_back("+", idxVar, "1", tStep, IRType::INT32);
    ir.emplace_back("=", tStep, "", idxVar, IRType::INT32);

    emitGoto(Lstart);
    emitLabel(Lend);
}

// ── Function IR helpers ───────────────────────────────────────────────────────
//
//  IR layout for a function:
//
//    func_begin  add  [i32]         ; function starts, return type in type field
//      param     x    [i32]         ; one per parameter
//      param     y    [i32]
//      <body instructions>
//      return    t5   [i32]         ; return value (or "" for void)
//    func_end    add                ; marks end
//
//  IR layout for a call site:
//
//    push_arg    a    [i32]         ; one per argument, left to right
//    push_arg    b    [i32]
//    call        add  ""   t7 [i32] ; result in t7 (or "" if void)
//
void emitFuncBegin(const std::string& name, IRType retType) {
    ir.emplace_back("func_begin", name, "", "", retType);
}

void emitFuncEnd(const std::string& name) {
    ir.emplace_back("func_end", name, "", "");
}

void emitParam(const std::string& name, IRType t) {
    ir.emplace_back("param", name, "", "", t);
}

void emitPushArg(const std::string& val, IRType t) {
    ir.emplace_back("push_arg", val, "", "", t);
}

void emitCall(const std::string& funcName,
              const std::vector<std::string>& args,
              const std::string& resultTemp,
              IRType retType)
{
    // Each argument is emitted as a push_arg before the call instruction.
    // The call instruction itself just names the function and the result temp.
    for (auto& a : args)
        ir.emplace_back("push_arg", a, "", "", IRType::UNKNOWN);
    ir.emplace_back("call", funcName, "", resultTemp, retType);
}

void emitReturn(const std::string& val, IRType t) {
    ir.emplace_back("return", val, "", "", t);
}

// ── Widening cast ─────────────────────────────────────────────────────────────
std::string emitCastIfNeeded(const std::string& val, IRType fromType, IRType toType) {
    if (fromType == toType
        || fromType == IRType::UNKNOWN
        || toType   == IRType::UNKNOWN)
        return val;

    std::string t = newTemp();
    IRInstruction ins("cast", val,
                      irTypeName(fromType) + "->" + irTypeName(toType),
                      t, toType);
    ins.srcType = fromType;
    ir.push_back(ins);
    return t;
}

// ── Format one instruction as human-readable text ────────────────────────────
static std::string fmtInstr(const IRInstruction& i) {
    if (i.op == "label")
        return i.arg1 + ":";

    if (i.op == "goto")
        return "    goto " + i.arg1;

    if (i.op == "ifzero_goto")
        return "    if " + i.arg1 + " == 0 goto " + i.arg2;

    if (i.op == "comment")
        return "    // " + i.arg1;

    if (i.op == "func_begin")
        return "\n" + i.arg1 + "  [" + irTypeName(i.type) + "] {";

    if (i.op == "func_end")
        return "}" + std::string("  // end ") + i.arg1 + "\n";

    if (i.op == "param")
        return "    param " + i.arg1 + "  [" + irTypeName(i.type) + "]";

    if (i.op == "push_arg")
        return "    push_arg " + i.arg1
               + (i.type != IRType::UNKNOWN ? ("  [" + irTypeName(i.type) + "]") : "");

    if (i.op == "call") {
        std::string s = "    call " + i.arg1;
        if (!i.result.empty()) s += "  ->  " + i.result;
        if (i.type != IRType::UNKNOWN) s += "  [" + irTypeName(i.type) + "]";
        return s;
    }

    if (i.op == "return") {
        std::string s = "    return";
        if (!i.arg1.empty()) s += " " + i.arg1;
        if (i.type != IRType::UNKNOWN) s += "  [" + irTypeName(i.type) + "]";
        return s;
    }

    auto typeTag = [](IRType t) -> std::string {
        return (t != IRType::UNKNOWN) ? ("  [" + irTypeName(t) + "]") : "";
    };

    if (i.op == "cast")
        return "    " + i.result + " = (" + irTypeName(i.type) + ") " + i.arg1
               + "  [" + irTypeName(i.srcType) + "->" + irTypeName(i.type) + "]";

    if (i.op == "=")
        return "    " + i.result + " = " + i.arg1 + typeTag(i.type);

    // binary / comparison
    return "    " + i.result + " = " + i.arg1 + " " + i.op + " " + i.arg2
           + typeTag(i.type);
}

// ── Stdout dump (debug) ───────────────────────────────────────────────────────
void printIR() {
    std::cout << "\n=== Generated IR ===\n";
    for (auto& ins : ir)
        std::cout << fmtInstr(ins) << "\n";
}

// ── File output ───────────────────────────────────────────────────────────────
void writeIRToFile(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) { std::cerr << "Error: cannot open " << filename << "\n"; return; }

    out << "; ── Three-Address IR ──────────────────────────────────────────────\n"
        << "; Format:  result = arg1 OP arg2  [type]\n"
        << "; Cast:    dest   = (toType) src   [srcType->toType]\n"
        << "; Types:   i32 / i64 / i128\n"
        << "; ─────────────────────────────────────────────────────────────────\n\n";

    for (auto& ins : ir)
        out << fmtInstr(ins) << "\n";

    out.close();
    std::cout << "[IR written to " << filename << "]\n";
}