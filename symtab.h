#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "ir.h"

// ── One symbol ────────────────────────────────────────────────────────────────
struct Symbol {
    std::string type;       // "int32", "float", etc.
    bool        isArray  = false;
    std::vector<int> dimensions;  // empty for scalars, {10} for 1D, {10,20} for 2D
    IRType      irType   = IRType::UNKNOWN;
};

// ── One function signature (stored separately from variables) ─────────────────
struct FuncSignature {
    IRType                   returnType;
    std::vector<IRType>      paramTypes;
    std::vector<std::string> paramNames;
};

// ── Symbol table with scope stack ─────────────────────────────────────────────
//
//  Scopes are pushed/popped like a stack:
//    enterScope()  — called when entering a function or block  { }
//    leaveScope()  — called when leaving
//
//  Lookup walks from innermost scope outward (standard lexical scoping).
//  Functions live in a separate flat table (they are always global).
//
class SymbolTable {
public:
    // ── Variable operations ───────────────────────────────────────────────────
    void   insert (const std::string& name,
                   const std::string& type,
                   const std::vector<int>& dimensions);

    bool   exists (const std::string& name) const;
    Symbol get    (const std::string& name) const;

    // ── Scope management ──────────────────────────────────────────────────────
    void enterScope();   // push a new empty scope
    void leaveScope();   // pop innermost scope (variables declared inside are gone)
    int  depth()  const; // 0 = global

    // ── Function operations ───────────────────────────────────────────────────
    void           insertFunc (const std::string& name, const FuncSignature& sig);
    bool           funcExists (const std::string& name) const;
    FuncSignature  getFunc    (const std::string& name) const;

    // ── Iteration — returns all variables in global scope (scope 0) ──────────
    const std::unordered_map<std::string, Symbol>& globalSymbols() const {
        return scopes[0];
    }

    // ── Debug ─────────────────────────────────────────────────────────────────
    void print() const;

private:
    // Each element of the stack is one scope: name → Symbol
    std::vector<std::unordered_map<std::string, Symbol>> scopes = {{}};

    // Function signatures — always global
    std::unordered_map<std::string, FuncSignature> funcs;
};

extern SymbolTable symtab;