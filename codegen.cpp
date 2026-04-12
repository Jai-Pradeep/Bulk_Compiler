#include "codegen.h"
#include "ir.h"
#include "symtab.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <cstdlib>

static std::string cType(IRType t) {
    switch (t) {
        case IRType::INT32:  return "int";
        case IRType::INT64:  return "long long";
        case IRType::INT128: return "__int128";
        case IRType::VOID:   return "void";
        default:             return "int";
    }
}

static bool isTemp(const std::string& s) {
    if (s.size() < 2 || s[0] != 't') return false;
    for (size_t i = 1; i < s.size(); ++i)
        if (!std::isdigit(s[i])) return false;
    return true;
}

static bool isLiteral(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    if (start == s.size()) return false;
    for (size_t i = start; i < s.size(); ++i)
        if (!std::isdigit(s[i])) return false;
    return true;
}

static bool isParallelComment(const std::string& s) {
    return s.find("PARALLEL LOOP") != std::string::npos;
        // && s.find("no loop-carried") != std::string::npos;
}

static std::unordered_map<std::string, IRType>
collectTemps(size_t start, size_t end) {
    std::unordered_map<std::string, IRType> temps;
    for (size_t i = start; i < end && i < ir.size(); ++i) {
        auto& ins = ir[i];
        auto rec = [&](const std::string& n, IRType t) {
            if (isTemp(n) && !temps.count(n))
                temps[n] = (t != IRType::UNKNOWN) ? t : IRType::INT32;
        };
        rec(ins.result, ins.type);
        rec(ins.arg1,   ins.type);
        rec(ins.arg2,   ins.type);
    }
    return temps;
}

// ── Detect a parallel loop block in the IR ───────────────────────────────────
//
//  A parallel loop looks like:
//    comment  "PARALLEL LOOP..."
//    =        idxVar   0           (init)
//    label    Lstart
//    <op>     idxVar   N    tBound (bound check)
//    ifzero   tBound   Lend
//    < body instructions >
//    +        idxVar   1    tStep  (increment)
//    =        tStep    ""   idxVar
//    goto     Lstart
//    label    Lend
//
//  We detect this pattern and emit a proper C for loop with #pragma omp.
//
struct ParallelLoop {
    bool   found    = false;
    size_t commentIdx;          // index of PARALLEL LOOP comment
    size_t initIdx;             // idxVar = 0
    size_t lstartIdx;           // label Lstart
    size_t boundIdx;            // tBound = idxVar < N
    size_t ifzeroIdx;           // ifzero tBound Lend
    size_t bodyStart;           // first body instruction
    size_t bodyEnd;             // index of increment instruction
    size_t gotoIdx;
    size_t lendIdx;
    std::string idxVar;
    std::string bound;          // N as string
    std::string Lstart, Lend;
};

static ParallelLoop detectParallelLoop(size_t commentPos) {
    ParallelLoop pl;
    size_t i = commentPos;

    // Skip any extra comment lines (e.g. "parallel array: ...")
    while (i < ir.size() && ir[i].op == "comment") ++i;

    // init: idxVar = 0
    if (i >= ir.size() || ir[i].op != "=") return pl;
    if (ir[i].arg1 != "0") return pl;
    pl.initIdx = i;
    pl.idxVar  = ir[i].result;
    ++i;

    // label Lstart
    if (i >= ir.size() || ir[i].op != "label") return pl;
    pl.lstartIdx = i;
    pl.Lstart    = ir[i].arg1;
    ++i;

    // tBound = idxVar < N
    if (i >= ir.size()) return pl;
    pl.boundIdx = i;
    pl.bound    = ir[i].arg2;
    std::string tBound = ir[i].result;
    ++i;

    // ifzero tBound Lend
    if (i >= ir.size() || ir[i].op != "ifzero_goto") return pl;
    if (ir[i].arg1 != tBound) return pl;
    pl.ifzeroIdx = i;
    pl.Lend      = ir[i].arg2;
    ++i;

    pl.bodyStart = i;

    // Scan forward for:  tStep = idxVar + 1
    while (i < ir.size()) {
        if (ir[i].op == "+"
            && ir[i].arg1 == pl.idxVar
            && ir[i].arg2 == "1") break;
        ++i;
    }
    if (i >= ir.size()) return pl;
    pl.bodyEnd = i;
    std::string tStep = ir[i].result;
    ++i;

    // idxVar = tStep
    if (i >= ir.size() || ir[i].op != "=" || ir[i].arg1 != tStep) return pl;
    ++i;

    // goto Lstart
    if (i >= ir.size() || ir[i].op != "goto" || ir[i].arg1 != pl.Lstart) return pl;
    pl.gotoIdx = i;
    ++i;

    // label Lend
    if (i >= ir.size() || ir[i].op != "label" || ir[i].arg1 != pl.Lend) return pl;
    pl.lendIdx = i;

    pl.found = true;
    return pl;
}

// ── Emit one IR instruction as C ─────────────────────────────────────────────
static std::string emitC(const IRInstruction& ins, const std::string& indent) {
    if (ins.op == "label")       return ins.arg1 + ":;";
    if (ins.op == "goto")        return indent + "goto " + ins.arg1 + ";";
    if (ins.op == "ifzero_goto") return indent + "if (!(" + ins.arg1 + ")) goto " + ins.arg2 + ";";
    if (ins.op == "comment")     return indent + "// " + ins.arg1;
    if (ins.op == "param")       return "";
    if (ins.op == "push_arg")    return "";
    if (ins.op == "return") {
        if (ins.arg1.empty()) return indent + "return;";
        return indent + "return " + ins.arg1 + ";";
    }
    if (ins.op == "cast")
        return indent + ins.result + " = (" + cType(ins.type) + ") " + ins.arg1 + ";";
    if (ins.op == "=")
        return indent + ins.result + " = " + ins.arg1 + ";";
    return indent + ins.result + " = " + ins.arg1 + " " + ins.op + " " + ins.arg2 + ";";
}

// ── Emit a range of IR as C, handling parallel loops ─────────────────────────
static void emitRange(std::ofstream& out,
                      size_t start, size_t end,
                      const std::string& indent)
{
    std::vector<std::string> pendingArgs;

    size_t i = start;
    while (i < end && i < ir.size()) {
        auto& ins = ir[i];

        // Detect parallel loop
        if (ins.op == "comment" && isParallelComment(ins.arg1)) {
            ParallelLoop pl = detectParallelLoop(i);
            if (pl.found) {
                // Emit as proper C for loop with OpenMP pragma
                out << "\n";
                out << indent << "// auto-parallelised by BulkCompiler\n";
                out << indent << "#pragma omp parallel for schedule(static)\n";
                out << indent << "for (int " << pl.idxVar << " = 0; "
                    << pl.idxVar << " < " << pl.bound << "; "
                    << "++" << pl.idxVar << ") {\n";

                // Emit body
                emitRange(out, pl.bodyStart, pl.bodyEnd, indent + "    ");

                out << indent << "}\n";

                // Skip past the whole loop pattern
                i = pl.lendIdx + 1;
                continue;
            }
            // Not a clean parallel loop — fall through to emit as comment
        }

        if (ins.op == "func_begin" || ins.op == "func_end" || ins.op == "param") {
            ++i; continue;
        }

        if (ins.op == "push_arg") {
            pendingArgs.push_back(ins.arg1);
            ++i; continue;
        }

        if (ins.op == "call") {
            std::string al;
            for (size_t a = 0; a < pendingArgs.size(); ++a) {
                if (a) al += ", ";
                al += pendingArgs[a];
            }
            pendingArgs.clear();
            if (ins.result.empty())
                out << indent << ins.arg1 << "(" << al << ");\n";
            else
                out << indent << ins.result << " = " << ins.arg1
                    << "(" << al << ");\n";
            ++i; continue;
        }

        std::string line = emitC(ins, indent);
        if (!line.empty()) out << line << "\n";
        ++i;
    }
}

// ── Main entry point ──────────────────────────────────────────────────────────
void generateCode(const std::string& cFile, const std::string& exeName) {

    // Pass 1: find function boundaries
    struct FuncRange { size_t begin, end; std::string name; IRType ret; };
    std::vector<FuncRange> funcs;
    std::set<size_t> inFuncIdx;

    for (size_t i = 0; i < ir.size(); ++i) {
        if (ir[i].op == "func_begin") {
            FuncRange fr;
            fr.name  = ir[i].arg1;
            fr.ret   = ir[i].type;
            fr.begin = i;
            for (size_t j = i+1; j < ir.size(); ++j) {
                if (ir[j].op == "func_end" && ir[j].arg1 == fr.name) {
                    fr.end = j;
                    funcs.push_back(fr);
                    i = j;
                    break;
                }
            }
        }
    }
    for (auto& fr : funcs)
        for (size_t k = fr.begin; k <= fr.end; ++k)
            inFuncIdx.insert(k);

    // Write C file
    std::ofstream out(cFile);
    if (!out) { std::cerr << "Error: cannot open " << cFile << "\n"; return; }

    out << "// Generated by BulkCompiler\n";
    out << "// Compile: gcc -O2 -fopenmp " << cFile << " -o " << exeName << "\n\n";
    out << "#include <stdio.h>\n#include <stdlib.h>\n#include <omp.h>\n\n";

    // Forward declarations
    for (auto& fr : funcs) {
        std::string params;
        for (size_t j = fr.begin+1; j < ir.size() && ir[j].op == "param"; ++j) {
            if (!params.empty()) params += ", ";
            params += cType(ir[j].type) + " " + ir[j].arg1;
        }
        out << cType(fr.ret) << " " << fr.name << "(" << params << ");\n";
    }
    if (!funcs.empty()) out << "\n";

    // Global variable declarations from symbol table
    for (auto& [name, sym] : symtab.globalSymbols()) {
        if (sym.isArray)
            out << cType(sym.irType) << " " << name << "[" << sym.size << "] = {0};\n";
        else
            out << cType(sym.irType) << " " << name << " = 0;\n";
    }
    out << "\n";

    // Function definitions
    for (auto& fr : funcs) {
        std::string params;
        size_t bodyStart = fr.begin + 1;
        while (bodyStart < ir.size() && ir[bodyStart].op == "param") {
            if (!params.empty()) params += ", ";
            params += cType(ir[bodyStart].type) + " " + ir[bodyStart].arg1;
            ++bodyStart;
        }
        out << cType(fr.ret) << " " << fr.name << "(" << params << ") {\n";
        auto temps = collectTemps(bodyStart, fr.end);
        for (auto& [name, type] : temps)
            out << "    " << cType(type) << " " << name << " = 0;\n";
        if (!temps.empty()) out << "\n";
        emitRange(out, bodyStart, fr.end, "    ");
        out << "}\n\n";
    }

    // main()
    out << "int main(int argc, char* argv[]) {\n";

    // Declare global-scope temps
    auto allTemps = collectTemps(0, ir.size());
    for (auto& fr : funcs) {
        auto ft = collectTemps(fr.begin, fr.end+1);
        for (auto& [k,v] : ft) allTemps.erase(k);
    }
    for (auto& [name, type] : allTemps)
        out << "    " << cType(type) << " " << name << " = 0;\n";
    if (!allTemps.empty()) out << "\n";

    // Emit global-scope IR (skip function bodies)
    // Build index list excluding func body indices
    emitRange(out, 0, ir.size(), "    ");
    // But emitRange will encounter func_begin/end and skip them — that's fine
    // However we need to skip indices that are inside functions
    // emitRange already skips func_begin/func_end/param instructions
    // The body instructions inside functions will also be encountered
    // Let's fix this properly:
    out.seekp(0); // can't seek in ofstream easily, so close and redo main
    out.close();

    // Reopen and redo main() section properly
    std::ofstream out2(cFile, std::ios::app);
    // We already wrote everything up to and including function defs
    // Now we need to redo main — but we've already written it partially
    // This is getting complex. Let's just build a clean string buffer.
    out2.close();

    // Clean approach: rebuild file from scratch with proper main
    std::ofstream final(cFile);
    final << "// Generated by BulkCompiler\n";
    final << "// Compile: gcc -O2 -fopenmp " << cFile << " -o " << exeName << "\n\n";
    final << "#include <stdio.h>\n#include <stdlib.h>\n#include <omp.h>\n\n";

    for (auto& fr : funcs) {
        std::string params;
        for (size_t j = fr.begin+1; j < ir.size() && ir[j].op == "param"; ++j) {
            if (!params.empty()) params += ", ";
            params += cType(ir[j].type) + " " + ir[j].arg1;
        }
        final << cType(fr.ret) << " " << fr.name << "(" << params << ");\n";
    }
    if (!funcs.empty()) final << "\n";

    for (auto& [name, sym] : symtab.globalSymbols()) {
        if (sym.isArray)
            final << cType(sym.irType) << " " << name << "[" << sym.size << "] = {0};\n";
        else
            final << cType(sym.irType) << " " << name << " = 0;\n";
    }
    final << "\n";

    for (auto& fr : funcs) {
        std::string params;
        size_t bodyStart = fr.begin + 1;
        while (bodyStart < ir.size() && ir[bodyStart].op == "param") {
            if (!params.empty()) params += ", ";
            params += cType(ir[bodyStart].type) + " " + ir[bodyStart].arg1;
            ++bodyStart;
        }
        final << cType(fr.ret) << " " << fr.name << "(" << params << ") {\n";
        auto temps = collectTemps(bodyStart, fr.end);
        for (auto& [name, type] : temps)
            final << "    " << cType(type) << " " << name << " = 0;\n";
        if (!temps.empty()) final << "\n";
        emitRange(final, bodyStart, fr.end, "    ");
        final << "}\n\n";
    }

    final << "int main(int argc, char* argv[]) {\n";

    // Declare only truly global-scope temps
    for (auto& [name, type] : allTemps)
        final << "    " << cType(type) << " " << name << " = 0;\n";
    if (!allTemps.empty()) final << "\n";

    // Emit only global-scope instructions
    // We need a version of emitRange that skips function bodies
    std::vector<size_t> globalIdxs;
    for (size_t i = 0; i < ir.size(); ++i)
        if (!inFuncIdx.count(i)) globalIdxs.push_back(i);

    // Write a filtered IR range using the index list
    std::vector<std::string> pendingArgs;
    size_t gi = 0;
    while (gi < globalIdxs.size()) {
        size_t i = globalIdxs[gi];
        auto& ins = ir[i];

        if (ins.op == "comment" && isParallelComment(ins.arg1)) {
            // Try to detect parallel loop using consecutive global indices
            ParallelLoop pl = detectParallelLoop(i);
            if (pl.found) {
                final << "\n";
                final << "    // auto-parallelised by BulkCompiler\n";
                final << "    #pragma omp parallel for schedule(static)\n";
                final << "    for (int " << pl.idxVar << " = 0; "
                      << pl.idxVar << " < " << pl.bound << "; "
                      << "++" << pl.idxVar << ") {\n";
                // Body
                emitRange(final, pl.bodyStart, pl.bodyEnd, "        ");
                final << "    }\n";
                // Skip all global indices up through lendIdx
                while (gi < globalIdxs.size() && globalIdxs[gi] <= pl.lendIdx) ++gi;
                continue;
            }
        }

        if (ins.op == "func_begin" || ins.op == "func_end" || ins.op == "param") {
            ++gi; continue;
        }
        if (ins.op == "push_arg") { pendingArgs.push_back(ins.arg1); ++gi; continue; }
        if (ins.op == "call") {
            std::string al;
            for (size_t a = 0; a < pendingArgs.size(); ++a) {
                if (a) al += ", ";
                al += pendingArgs[a];
            }
            pendingArgs.clear();
            if (ins.result.empty())
                final << "    " << ins.arg1 << "(" << al << ");\n";
            else
                final << "    " << ins.result << " = " << ins.arg1 << "(" << al << ");\n";
            ++gi; continue;
        }

        std::string line = emitC(ins, "    ");
        if (!line.empty()) final << line << "\n";
        ++gi;
    }

    final << "    return 0;\n}\n";
    final.close();

    std::cout << "[Codegen] C file written to: " << cFile << "\n";

    std::string cmd = "gcc -O2 -fopenmp " + cFile + " -o " + exeName + " 2>&1";
    std::cout << "[Codegen] Compiling: " << cmd << "\n";
    int ret = system(cmd.c_str());
    if (ret == 0)
        std::cout << "[Codegen] Success! Run with: ./" << exeName << "\n";
    else
        std::cerr << "[Codegen] gcc failed — see errors above\n";
}
