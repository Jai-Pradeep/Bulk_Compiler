#pragma once
#include "ast.h"
#include <string>
#include <set>
#include <map>

// ── Dependency analysis result ────────────────────────────────────────────────
enum class ParallelKind {
    FULLY_PARALLEL,      // All iterations fully independent — safe to SIMD/thread
    PARTIAL_PARALLEL,    // Some array accesses independent, others are not
    NOT_PARALLEL,        // Loop-carried dependency detected — must run sequentially
    UNKNOWN              // Could not determine statically
};

struct DepReport {
    ParallelKind kind;

    // Which arrays were found to be safe (index == loop var, no offset)
    std::set<std::string> parallelArrays;

    // Which arrays caused a dependency and why
    std::map<std::string, std::string> blockers;  // array -> reason

    std::string loopVar;   // the loop index variable (e.g. "i")

    std::string summary() const;
};

// ── Walk an AST body and collect array access info ────────────────────────────

struct ArrayAccess {
    std::string array;
    std::string indexExpr;  // simplified string form of the index
    bool        isWrite;
};

// Recursively collect all array reads and writes from a statement list
void collectAccesses(ASTNode* node, const std::string& loopVar,
                     std::vector<ArrayAccess>& out);

// Simplify an index ASTNode to a string relative to the loop var
// e.g.  i       -> "+0"
//       i+1     -> "+1"
//       i-1     -> "-1"
//       j       -> "other_var"  (not the loop var)
//       constant -> "const"
std::string classifyIndex(ASTNode* index, const std::string& loopVar);

// ── Main entry point ──────────────────────────────────────────────────────────
// Analyse the body of a for loop whose index variable is loopVar.
DepReport analyzeLoop(ASTNode* body, const std::string& loopVar);