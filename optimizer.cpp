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
static bool isTemp(const std::string& s) {
    if (s.size() < 2 || s[0] != 't') return false;
    for (size_t i = 1; i < s.size(); ++i)
        if (!std::isdigit(s[i])) return false;
    return true;
}

static bool hasSideEffect(const std::string& op) {
    return op=="label"     || op=="goto"      || op=="ifzero_goto"
        || op=="call"      || op=="return"    || op=="push_arg"
        || op=="func_begin"|| op=="func_end"  || op=="param"
        || op=="comment";
}

static bool deadCodeElimination() {
    std::unordered_set<std::string> used;
    for (auto& ins : ir) {
        if (!ins.arg1.empty() && isTemp(ins.arg1)) used.insert(ins.arg1);
        if (!ins.arg2.empty() && isTemp(ins.arg2)) used.insert(ins.arg2);
        if (ins.op == "call" && isTemp(ins.result)) used.insert(ins.result);
    }

    size_t before = ir.size();
    std::vector<IRInstruction> kept;
    kept.reserve(ir.size());
    for (auto& ins : ir) {
        if (!hasSideEffect(ins.op) && isTemp(ins.result) && !used.count(ins.result))
            continue;   // dead — drop it
        kept.push_back(ins);
    }
    ir = std::move(kept);
    return ir.size() < before;
}

static bool isBinaryOp(const std::string& op) {
    return op == "+" || op == "-" || op == "*" || op == "/" ||
           op == "<" || op == ">" || op == "<=" || op == ">=" ||
           op == "==" || op == "!=";
}

static bool commonSubexpressionElimination() {
    bool changed = false;

    std::unordered_map<std::string, std::string> exprMap;
    // key -> result temp

    for (auto& ins : ir) {

        // Reset at control flow boundaries
        if (ins.op == "label" ||
            ins.op == "goto" ||
            ins.op == "ifzero_goto" ||
            ins.op == "func_begin" ||
            ins.op == "func_end") {
            exprMap.clear();
            continue;
        }

        // Only consider binary operations
        if (!isBinaryOp(ins.op)) {
            // If variable is redefined → invalidate expressions using it
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

        // Build key
        std::string key = ins.op + "|" + ins.arg1 + "|" + ins.arg2;

        // Check if already computed
        if (exprMap.count(key)) {
            // Replace with copy
            ins.op = "=";
            ins.arg1 = exprMap[key];
            ins.arg2 = "";
            changed = true;
        } else {
            exprMap[key] = ins.result;
        }
    }

    return changed;
}

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

        // Detect loop start
        if (ir[i].op != "label") continue;
        std::string Lstart = ir[i].arg1;

        // Find back-edge
        size_t loopEnd = i + 1;
        bool found = false;

        for (; loopEnd < ir.size(); ++loopEnd) {
            if (ir[loopEnd].op == "goto" &&
                ir[loopEnd].arg1 == Lstart) {
                found = true;
                break;
            }
        }

        if (!found) continue;

        size_t loopStart = i + 1;

        // 🔥 Step 0: detect loop variable
        std::string loopVar;
        for (size_t j = i; j < loopStart && j < ir.size(); ++j) {
            if (ir[j].op == "=" && ir[j].arg1 == "0") {
                loopVar = ir[j].result;
                break;
            }
        }

        if (loopVar.empty()) continue; // safety

        // Step 1: collect modified vars
        std::set<std::string> modified;
        for (size_t j = loopStart; j < loopEnd; ++j) {
            if (!ir[j].result.empty())
                modified.insert(ir[j].result);
        }

        // Step 2: find invariants
        std::vector<size_t> invariants;

        for (size_t j = loopStart; j < loopEnd; ++j) {
            auto& ins = ir[j];

            if (!isSafeOp(ins.op)) continue;

            // Skip if result is modified later (conservative)
            if (modified.count(ins.result)) continue;

            // Check operands
            bool arg1_ok =
                (isConstant(ins.arg1) ||
                (!modified.count(ins.arg1) && !usesVar(ins.arg1, loopVar)));

            bool arg2_ok =
                (isConstant(ins.arg2) ||
                (!modified.count(ins.arg2) && !usesVar(ins.arg2, loopVar)));

            // 🚫 CRITICAL: reject array accesses like a[i]
            if (usesVar(ins.arg1, loopVar) || usesVar(ins.arg2, loopVar))
                continue;

            if (arg1_ok && arg2_ok) {
                invariants.push_back(j);
            }
        }

        if (invariants.empty()) continue;

        // Step 3: hoist
        size_t insertPos = i;

        std::vector<IRInstruction> hoisted;
        for (auto idx : invariants)
            hoisted.push_back(ir[idx]);

        // Remove from loop (reverse order)
        for (auto it = invariants.rbegin(); it != invariants.rend(); ++it) {
            ir.erase(ir.begin() + *it);
            changed = true;
        }

        // Insert before loop
        ir.insert(ir.begin() + insertPos, hoisted.begin(), hoisted.end());

        // Move index forward
        i = loopEnd;
    }

    return changed;
}

// ── Entry point ───────────────────────────────────────────────────────────────

void optimizeIR(int level) {
    size_t before = ir.size();
    printf("  IR instructions before : %zu\n", before);

    if (level == 1) {
        // One round of all three passes
        constantFolding();
        copyPropagation();
        commonSubexpressionElimination();
        loopInvariantCodeMotion(); 
        deadCodeElimination();
        printf("  Passes run             : folding, propagation, dead-code (1 round)\n");

    } else if (level >= 2) {
        // Repeat folding + propagation until nothing changes, then DCE once
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
