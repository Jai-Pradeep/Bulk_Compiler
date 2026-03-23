#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include "ast.h"

enum class ParallelKind {
    FULLY_PARALLEL,
    PARTIAL_PARALLEL,
    NOT_PARALLEL,
    UNKNOWN
};

struct ArrayAccess {
    std::string array;
    std::string indexExpr;
    bool        isWrite;
};

struct DepReport {
    std::string              loopVar;
    ParallelKind             kind = ParallelKind::UNKNOWN;
    std::set<std::string>    parallelArrays;
    std::map<std::string, std::string> blockers;
    std::string summary() const;
};

std::string classifyIndex(ASTNode* index, const std::string& loopVar);
void        collectAccesses(ASTNode* node, const std::string& loopVar,
                            std::vector<ArrayAccess>& out);
DepReport   analyzeLoop(ASTNode* body, const std::string& loopVar);