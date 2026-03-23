#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "ir.h"

// ── One symbol ────────────────────────────────────────────────────────────────
struct Symbol {
    std::string type;       // "int32", "int64", "int128", "void"
    bool        isArray  = false;
    int         size     = 0;
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
                   bool isArray, int size);

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

    // ── Debug ─────────────────────────────────────────────────────────────────
    void print() const;

private:
    // Each element of the stack is one scope: name → Symbol
    std::vector<std::unordered_map<std::string, Symbol>> scopes = {{}};

    // Function signatures — always global
    std::unordered_map<std::string, FuncSignature> funcs;
};

extern SymbolTable symtab;