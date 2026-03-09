#include "ir.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

// ── Globals 
std::vector<IRInstruction> ir;
int tempCount = 0;

// ── Type helpers 
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

// ── IRInstruction ctor 
IRInstruction::IRInstruction(std::string o, std::string a1, std::string a2,
                             std::string r, IRType t)
    : op(o), arg1(a1), arg2(a2), result(r), type(t) {}

// ── Name generators 
std::string newTemp()  { return "t" + std::to_string(++tempCount); }
std::string newLabel() { return "L" + std::to_string(++tempCount); }

// ── Emit helpers (everything goes into the `ir` vector) 
void emitLabel  (const std::string& label)                      { ir.emplace_back("label",        label, "",    ""); }
void emitGoto   (const std::string& label)                      { ir.emplace_back("goto",         label, "",    ""); }
void emitIfZero (const std::string& cond, const std::string& lbl){ ir.emplace_back("ifzero_goto",  cond,  lbl,  ""); }
void emitForLoop(const std::string& idx, int size)              { ir.emplace_back("comment", "FOR " + idx + " = 0 .. " + std::to_string(size-1), "", ""); }
void emitIf     (const std::string& cond)                       { ir.emplace_back("comment", "IF "      + cond, "", ""); }
void endIf      ()                                              { ir.emplace_back("comment", "END IF",           "", ""); }
void endForLoop ()                                              { ir.emplace_back("comment", "END FOR",          "", ""); }

// ── Widening cast 
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

// ── Format one instruction as human-readable text 
static std::string fmtInstr(const IRInstruction& i) {
    if (i.op == "label")
        return i.arg1 + ":";

    if (i.op == "goto")
        return "    goto " + i.arg1;

    if (i.op == "ifzero_goto")
        return "    if " + i.arg1 + " == 0 goto " + i.arg2;

    if (i.op == "comment")
        return "    // " + i.arg1;

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

// ── Stdout dump (debug) 
void printIR() {
    std::cout << "\n=== Generated IR ===\n";
    for (auto& ins : ir)
        std::cout << fmtInstr(ins) << "\n";
}

// ── File output 
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