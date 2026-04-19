#include "depcheck.h"
#include <sstream>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
//  classifyIndex
//  Returns a canonical string describing how the index relates to the loop var:
//    "+0"        exact loop var  (a[i])
//    "+1"/"-1"   loop var with constant offset (a[i+1], a[i-1])
//    "const"     compile-time constant (a[5])
//    "other"     involves a variable that is not the loop var
// ─────────────────────────────────────────────────────────────────────────────
std::string classifyIndex(ASTNode* index, const std::string& loopVar) {
    if (!index) return "unknown";

    // Constant literal
    if (NumberNode* n = dynamic_cast<NumberNode*>(index))
        return "const:" + std::to_string(n->value);

    // Bare identifier
    if (IdentifierNode* id = dynamic_cast<IdentifierNode*>(index)) {
        if (id->name == loopVar) return "+0";
        return "other:" + id->name;
    }

    // Binary expression
    if (BinaryOpNode* bin = dynamic_cast<BinaryOpNode*>(index)) {
        std::string L = classifyIndex(bin->left,  loopVar);
        std::string R = classifyIndex(bin->right, loopVar);

        // i + constant
        if (L == "+0" && bin->op == "+") {
            if (NumberNode* n = dynamic_cast<NumberNode*>(bin->right))
                return "+" + std::to_string(n->value);
        }
        // i - constant
        if (L == "+0" && bin->op == "-") {
            if (NumberNode* n = dynamic_cast<NumberNode*>(bin->right))
                return "-" + std::to_string(n->value);
        }
        // constant + i
        if (R == "+0" && bin->op == "+") {
            if (NumberNode* n = dynamic_cast<NumberNode*>(bin->left))
                return "+" + std::to_string(n->value);
        }
        // i + i  or complex — treat as other
        // If both sides involve loopVar it's something like i*2 — not simple
        if (L.find("other") != std::string::npos ||
            R.find("other") != std::string::npos)
            return "other:complex";

        return "complex";
    }

    return "unknown";
}

// ─────────────────────────────────────────────────────────────────────────────
//  collectAccesses  — recursive walk of the body AST
// ─────────────────────────────────────────────────────────────────────────────
void collectAccesses(ASTNode* node, const std::string& loopVar,
                     std::vector<ArrayAccess>& out)
{
    if (!node) return;

    // Array element write:  a[i] = expr
    if (ArrayElementAssignmentNode* ae =
            dynamic_cast<ArrayElementAssignmentNode*>(node))
    {
        ArrayAccess w;
        w.array     = ae->name;
        w.indexExpr = classifyIndex(ae->indices[0], loopVar);
        w.isWrite   = true;
        out.push_back(w);
        // Also walk the RHS for reads
        collectAccesses(ae->expr, loopVar, out);
        return;
    }

    // Scalar assignment: x = expr  — walk expr for reads, x is scalar (ignore)
    if (AssignmentNode* a = dynamic_cast<AssignmentNode*>(node)) {
        collectAccesses(a->expr, loopVar, out);
        return;
    }

    // Array read:  a[i]
    if (ArrayAccessNode* ar = dynamic_cast<ArrayAccessNode*>(node)) {
        ArrayAccess r;
        r.array     = ar->name;
        r.indexExpr = classifyIndex(ar->indices[0], loopVar);
        r.isWrite   = false;
        out.push_back(r);
        return;
    }

    // Binary op — walk both sides
    if (BinaryOpNode* bin = dynamic_cast<BinaryOpNode*>(node)) {
        collectAccesses(bin->left,  loopVar, out);
        collectAccesses(bin->right, loopVar, out);
        return;
    }

    // Comparison — walk both sides
    if (ComparisonNode* cmp = dynamic_cast<ComparisonNode*>(node)) {
        collectAccesses(cmp->left,  loopVar, out);
        collectAccesses(cmp->right, loopVar, out);
        return;
    }

    // Statement list — walk each statement
    if (StatementListNode* sl = dynamic_cast<StatementListNode*>(node)) {
        for (auto* s : sl->stmts)
            collectAccesses(s, loopVar, out);
        return;
    }

    // If node — walk condition + both bodies
    if (IfNode* ifn = dynamic_cast<IfNode*>(node)) {
        collectAccesses(ifn->condition, loopVar, out);
        collectAccesses(ifn->thenBody,  loopVar, out);
        collectAccesses(ifn->elseBody,  loopVar, out);
        return;
    }

    // Nested for — treat its body conservatively as opaque (unknown)
    if (ForNode* fn = dynamic_cast<ForNode*>(node)) {
        // We don't recurse into nested loops here — they get their own analysis
        (void)fn;
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  analyzeLoop
//
//  Rules applied (all standard loop-carried dependency checks):
//
//  1. WRITE to a[i+k] where k != 0  →  RAW/WAR hazard with a[i] reads
//     e.g. a[i] = a[i-1] + 1   →  iteration i reads what iteration i-1 wrote
//
//  2. Same array READ at [j] (different var) and WRITTEN at [i]
//     e.g. a[i] = b[j]  where j is unrelated  →  unknown, treat as NOT_PARALLEL
//
//  3. Same array READ at [i+k] and WRITTEN at [i]  →  NOT_PARALLEL
//     e.g. a[i] = a[i+1]  (reads ahead)
//
//  4. All reads and writes use only [i] ("+0") on each array  →  FULLY_PARALLEL
//     e.g. a[i] = b[i] + c[i]
//
//  5. No array accesses at all  →  UNKNOWN (scalar-only loop, not our concern)
// ─────────────────────────────────────────────────────────────────────────────
DepReport analyzeLoop(ASTNode* body, const std::string& loopVar) {
    DepReport report;
    report.loopVar = loopVar;

    std::vector<ArrayAccess> accesses;
    collectAccesses(body, loopVar, accesses);

    if (accesses.empty()) {
        report.kind = ParallelKind::UNKNOWN;
        return report;
    }

    // Build per-array sets of index expressions (split read/write)
    std::map<std::string, std::set<std::string>> readIdx, writeIdx;
    for (auto& a : accesses) {
        if (a.isWrite) writeIdx[a.array].insert(a.indexExpr);
        else           readIdx [a.array].insert(a.indexExpr);
    }

    bool anyBlocked  = false;
    bool anyParallel = false;

    // Check every array that is written
    for (auto& [arr, widxSet] : writeIdx) {
        bool arrayOk = true;
        std::string reason;

        for (auto& widx : widxSet) {
            // Write index must be exactly "+0" (a[i]) for safe parallel write
            if (widx != "+0") {
                reason = "write to " + arr + "[i" + widx + "] — offset write is not safe";
                arrayOk = false;
                break;
            }
        }

        if (arrayOk) {
            // Check if any read of the same array uses a different index
            if (readIdx.count(arr)) {
                for (auto& ridx : readIdx[arr]) {
                    if (ridx != "+0") {
                        // e.g. a[i] = a[i-1] — loop-carried RAW dependency
                        reason = "read " + arr + "[i" + (ridx[0]=='+' ? ridx : ridx)
                                 + "] while writing " + arr + "[i]"
                                 " — loop-carried dependency";
                        arrayOk = false;
                        break;
                    }
                }
            }
        }

        // Also check reads of OTHER arrays for non-loop-var indices
        if (arrayOk) {
            for (auto& [otherArr, ridxSet] : readIdx) {
                if (otherArr == arr) continue;
                for (auto& ridx : ridxSet) {
                    if (ridx.substr(0,5) == "other") {
                        reason = "read " + otherArr + " with non-loop index '" + ridx
                                 + "' — cannot verify independence";
                        arrayOk = false;
                        break;
                    }
                }
                if (!arrayOk) break;
            }
        }

        if (arrayOk) {
            report.parallelArrays.insert(arr);
            anyParallel = true;
        } else {
            report.blockers[arr] = reason;
            anyBlocked = true;
        }
    }

    // Also flag read-only arrays with non-[i] indices as a warning
    for (auto& [arr, ridxSet] : readIdx) {
        if (writeIdx.count(arr)) continue;  // already handled above
        for (auto& ridx : ridxSet) {
            if (ridx.substr(0,5) == "other" || ridx == "complex") {
                report.blockers[arr] = "read " + arr + " with non-loop index — unknown access pattern";
                anyBlocked = true;
            }
        }
    }

    if      (!anyBlocked && anyParallel)  report.kind = ParallelKind::FULLY_PARALLEL;
    else if ( anyBlocked && anyParallel)  report.kind = ParallelKind::PARTIAL_PARALLEL;
    else if ( anyBlocked && !anyParallel) report.kind = ParallelKind::NOT_PARALLEL;
    else                                  report.kind = ParallelKind::UNKNOWN;

    return report;
}

// ─────────────────────────────────────────────────────────────────────────────
//  DepReport::summary()
// ─────────────────────────────────────────────────────────────────────────────
std::string DepReport::summary() const {
    std::ostringstream s;
    switch (kind) {
        case ParallelKind::FULLY_PARALLEL:
            s << "FULLY PARALLEL — all iterations independent";
            if (!parallelArrays.empty()) {
                s << " [arrays: ";
                for (auto& a : parallelArrays) s << a << " ";
                s << "]";
            }
            break;
        case ParallelKind::PARTIAL_PARALLEL:
            s << "PARTIALLY PARALLEL";
            if (!parallelArrays.empty()) {
                s << " — safe arrays: ";
                for (auto& a : parallelArrays) s << a << " ";
            }
            s << " — BLOCKED by: ";
            for (auto& [arr, reason] : blockers)
                s << "\n      " << arr << ": " << reason;
            break;
        case ParallelKind::NOT_PARALLEL:
            s << "NOT PARALLEL — loop-carried dependencies detected:";
            for (auto& [arr, reason] : blockers)
                s << "\n      " << arr << ": " << reason;
            break;
        case ParallelKind::UNKNOWN:
            s << "UNKNOWN — no array accesses found or pattern too complex";
            break;
    }
    return s.str();
}