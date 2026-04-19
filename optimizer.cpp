#include "optimizer.h"
#include "ir.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <iostream>
#include <set>

// ── Helpers ───────────────────────────────────────────────────────────────────

static bool isConstant(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    if (start == s.size()) return false;
    for (size_t i = start; i < s.size(); ++i)
        if (!std::isdigit(s[i])) return false;
    return true;
}

static long long toInt(const std::string& s) { return std::stoll(s); }

static bool isFoldableOp(const std::string& op) {
    return op=="+" || op=="-" || op=="*" || op=="/"
        || op=="<" || op==">" || op=="<=" || op==">="
        || op=="==" || op=="!=";
}

static long long foldBinary(const std::string& op, long long L, long long R) {
    if (op == "+")  return L + R;
    if (op == "-")  return L - R;
    if (op == "*")  return L * R;
    if (op == "/") {
        if (R == 0) { std::cerr << "Warning: division by zero in constant fold\n"; return 0; }
        return L / R;
    }
    if (op == "<")  return L <  R ? 1 : 0;
    if (op == ">")  return L >  R ? 1 : 0;
    if (op == "<=") return L <= R ? 1 : 0;
    if (op == ">=") return L >= R ? 1 : 0;
    if (op == "==") return L == R ? 1 : 0;
    if (op == "!=") return L != R ? 1 : 0;
    return 0;
}

// ── Extract ALL temp names from a string field ────────────────────────────────
// Handles plain temps ("t13") AND array-indexed notation ("arr[t13]").
// Scans for every occurrence of 't' followed by digits anywhere in the string.
static bool isTemp(const std::string& s) {
    if (s.size() < 2 || s[0] != 't') return false;
    for (size_t i = 1; i < s.size(); ++i)
        if (!std::isdigit(s[i])) return false;
    return true;
}

static void extractTemps(const std::string& field,
                          std::unordered_set<std::string>& used) {
    if (field.empty()) return;

    // Case 1: the whole field is a plain temp — fast path
    if (isTemp(field)) { used.insert(field); return; }

    // Case 2: scan for embedded temps, e.g. inside "arr[t13]" or "mat[i][t7]"
    size_t n = field.size();
    for (size_t i = 0; i < n; ++i) {
        if (field[i] == 't' && i + 1 < n && std::isdigit(field[i + 1])) {
            // make sure it's not the tail of a longer identifier
            if (i > 0 && (std::isalpha(field[i-1]) || std::isdigit(field[i-1]) || field[i-1] == '_'))
                continue;
            size_t j = i + 1;
            while (j < n && std::isdigit(field[j])) ++j;
            // make sure it ends at a non-identifier character
            if (j < n && (std::isalpha(field[j]) || field[j] == '_'))
                continue;
            used.insert(field.substr(i, j - i));
        }
    }
}

// ── Pass 1: Constant Folding ──────────────────────────────────────────────────
static bool constantFolding() {
    bool changed = false;
    for (auto& ins : ir) {
        if (!isFoldableOp(ins.op))        continue;
        if (ins.arg1.empty())             continue;
        if (ins.arg2.empty())             continue;
        if (!isConstant(ins.arg1))        continue;
        if (!isConstant(ins.arg2))        continue;

        long long V = foldBinary(ins.op, toInt(ins.arg1), toInt(ins.arg2));
        ins.arg1    = std::to_string(V);
        ins.arg2    = "";
        ins.op      = "=";
        changed     = true;
    }
    return changed;
}

// ── Pass 2: Copy Propagation ──────────────────────────────────────────────────
static bool copyPropagation() {
    bool changed = false;
    std::unordered_map<std::string, std::string> copyMap;

    auto substitute = [&](std::string& operand) {
        if (operand.empty()) return;
        auto it = copyMap.find(operand);
        if (it != copyMap.end()) { operand = it->second; changed = true; }
    };

    for (auto& ins : ir) {
        if (ins.op == "label" ||
            ins.op == "func_begin" ||
            ins.op == "func_end") {
            copyMap.clear();
            continue;
        }

        substitute(ins.arg1);
        substitute(ins.arg2);

        if (ins.op == "=" && !ins.result.empty() && !ins.arg1.empty() && ins.arg2.empty()) {
            for (auto it = copyMap.begin(); it != copyMap.end(); )
                if (it->second == ins.result) it = copyMap.erase(it); else ++it;
            copyMap[ins.result] = ins.arg1;
        } else if (!ins.result.empty()) {
            copyMap.erase(ins.result);
            for (auto it = copyMap.begin(); it != copyMap.end(); )
                if (it->second == ins.result) it = copyMap.erase(it); else ++it;
        }
    }
    return changed;
}

// ── Pass 3: Dead Code Elimination ────────────────────────────────────────────

static bool hasSideEffect(const std::string& op) {
    return op=="label"     || op=="goto"      || op=="ifzero_goto"
        || op=="call"      || op=="return"    || op=="push_arg"
        || op=="func_begin"|| op=="func_end"  || op=="param"
        || op=="scan"      || op=="print"     || op=="println"
        || op=="comment";
}

// ── Array store ops: result contains something like "arr[tN]" ────────────────
// These have side effects (memory write) and must never be dropped.
static bool isArrayStore(const IRInstruction& ins) {
    // Array stores are emitted as op="=" with result like "arr[i]" or "mat[i][j]"
    return !ins.result.empty() && ins.result.find('[') != std::string::npos;
}

static bool deadCodeElimination() {
    // ── Build the used-temp set ───────────────────────────────────────────────
    // We must scan EVERY field of every instruction, extracting temps even when
    // they appear as array indices embedded in strings like "arr[t13]".
    std::unordered_set<std::string> used;

    for (auto& ins : ir) {
        // Plain operand fields
        extractTemps(ins.arg1,   used);
        extractTemps(ins.arg2,   used);
        extractTemps(ins.result, used);   // e.g. result = "arr[t16]" for stores

        // call result is always considered used (side-effectful)
        if (ins.op == "call" && isTemp(ins.result))
            used.insert(ins.result);
    }

    // ── Sweep: drop instructions whose result temp is never used ─────────────
    size_t before = ir.size();
    std::vector<IRInstruction> kept;
    kept.reserve(ir.size());

    for (auto& ins : ir) {
        // Never drop instructions with side effects
        if (hasSideEffect(ins.op)) { kept.push_back(ins); continue; }

        // Never drop array stores (memory writes) — they are side-effectful
        // even though they don't produce a temp result
        if (isArrayStore(ins))     { kept.push_back(ins); continue; }

        // Drop only if: result is a temp AND that temp is never used
        if (isTemp(ins.result) && !used.count(ins.result))
            continue;   // dead — drop it

        kept.push_back(ins);
    }

    ir = std::move(kept);
    return ir.size() < before;
}

// ── Pass 4: Common Subexpression Elimination ──────────────────────────────────
static bool isBinaryOp(const std::string& op) {
    return op == "+" || op == "-" || op == "*" || op == "/" ||
           op == "<" || op == ">" || op == "<=" || op == ">=" ||
           op == "==" || op == "!=";
}

static bool commonSubexpressionElimination() {
    bool changed = false;
    std::unordered_map<std::string, std::string> exprMap;

    for (auto& ins : ir) {
        if (ins.op == "label"      ||
            ins.op == "goto"       ||
            ins.op == "ifzero_goto"||
            ins.op == "func_begin" ||
            ins.op == "func_end") {
            exprMap.clear();
            continue;
        }

        if (!isBinaryOp(ins.op)) {
            if (!ins.result.empty()) {
                for (auto it = exprMap.begin(); it != exprMap.end(); ) {
                    if (it->first.find("|" + ins.result) != std::string::npos ||
                        it->first.find(ins.result + "|") != std::string::npos)
                        it = exprMap.erase(it);
                    else ++it;
                }
            }
            continue;
        }

        std::string key = ins.op + "|" + ins.arg1 + "|" + ins.arg2;
        if (exprMap.count(key)) {
            ins.op   = "=";
            ins.arg1 = exprMap[key];
            ins.arg2 = "";
            changed  = true;
        } else {
            exprMap[key] = ins.result;
        }
    }
    return changed;
}

// ── Pass 5: Loop-Invariant Code Motion ───────────────────────────────────────
static bool isSafeOp(const std::string& op) {
    return op == "+" || op == "-" || op == "*" || op == "/";
}

static bool usesVar(const std::string& expr, const std::string& var) {
    if (var.empty()) return false;
    return expr.find(var) != std::string::npos;
}

static bool loopInvariantCodeMotion() {
    bool changed = false;

    for (size_t i = 0; i < ir.size(); ++i) {
        if (ir[i].op != "label") continue;
        std::string Lstart = ir[i].arg1;

        size_t loopEnd = i + 1;
        bool found = false;
        for (; loopEnd < ir.size(); ++loopEnd) {
            if (ir[loopEnd].op == "goto" && ir[loopEnd].arg1 == Lstart) {
                found = true; break;
            }
        }
        if (!found) continue;

        size_t loopStart = i + 1;

        std::string loopVar;
        for (size_t j = i; j < loopStart && j < ir.size(); ++j) {
            if (ir[j].op == "=" && ir[j].arg1 == "0") {
                loopVar = ir[j].result; break;
            }
        }
        if (loopVar.empty()) continue;

        std::set<std::string> modified;
        for (size_t j = loopStart; j < loopEnd; ++j)
            if (!ir[j].result.empty()) modified.insert(ir[j].result);

        std::vector<size_t> invariants;
        for (size_t j = loopStart; j < loopEnd; ++j) {
            auto& ins = ir[j];
            if (!isSafeOp(ins.op)) continue;
            if (modified.count(ins.result)) continue;
            bool arg1_ok = isConstant(ins.arg1) ||
                           (!modified.count(ins.arg1) && !usesVar(ins.arg1, loopVar));
            bool arg2_ok = isConstant(ins.arg2) ||
                           (!modified.count(ins.arg2) && !usesVar(ins.arg2, loopVar));
            if (usesVar(ins.arg1, loopVar) || usesVar(ins.arg2, loopVar)) continue;
            if (arg1_ok && arg2_ok) invariants.push_back(j);
        }
        if (invariants.empty()) continue;

        size_t insertPos = i;
        std::vector<IRInstruction> hoisted;
        for (auto idx : invariants) hoisted.push_back(ir[idx]);
        for (auto it = invariants.rbegin(); it != invariants.rend(); ++it) {
            ir.erase(ir.begin() + *it);
            changed = true;
        }
        ir.insert(ir.begin() + insertPos, hoisted.begin(), hoisted.end());
        i = loopEnd;
    }
    return changed;
}

// ── Entry point ───────────────────────────────────────────────────────────────
void optimizeIR(int level) {
    size_t before = ir.size();
    printf("  IR instructions before : %zu\n", before);

    if (level == 1) {
        constantFolding();
        copyPropagation();
        commonSubexpressionElimination();
        loopInvariantCodeMotion();
        deadCodeElimination();
        printf("  Passes run             : folding, propagation, dead-code (1 round)\n");

    } else if (level >= 2) {
        int rounds = 0;
        bool changed = true;
        while (changed) {
            ++rounds;
            bool f = constantFolding();
            bool p = copyPropagation();
            bool c = commonSubexpressionElimination();
            bool l = loopInvariantCodeMotion();
            changed = f || p || c || l;
        }
        deadCodeElimination();
        printf("  Passes run             : folding+propagation (%d rounds) + dead-code\n", rounds);
    }

    size_t after = ir.size();
    printf("  IR instructions after  : %zu", after);
    if (before > after)
        printf("  (removed %zu)\n", before - after);
    else
        printf("  (no change)\n");
}