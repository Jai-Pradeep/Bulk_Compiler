#include "optimizer.h"
#include "ir.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <iostream>

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

// ── Entry point ───────────────────────────────────────────────────────────────

void optimizeIR(int level) {
    size_t before = ir.size();
    printf("  IR instructions before : %zu\n", before);

    if (level == 1) {
        // One round of all three passes
        constantFolding();
        copyPropagation();
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
            changed = f || p;
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