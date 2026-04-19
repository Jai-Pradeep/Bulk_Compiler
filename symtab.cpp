#include "symtab.h"
#include "ir.h"
#include <iostream>

SymbolTable symtab;

void SymbolTable::insert(const std::string& name,
                         const std::string& type,
                         const std::vector<int>& dimensions)
{
    auto& current = scopes.back();
    if (current.count(name)) {
        std::cerr << "Error: redeclaration of '" << name << "' in the same scope\n";
        exit(1);
    }
    Symbol s;
    s.type       = type;
    s.isArray    = !dimensions.empty();
    s.dimensions = dimensions;
    s.irType     = parseType(type);
    current[name] = s;
}

bool SymbolTable::exists(const std::string& name) const {
    for (int i = (int)scopes.size() - 1; i >= 0; --i)
        if (scopes[i].count(name)) return true;
    return false;
}

Symbol SymbolTable::get(const std::string& name) const {
    for (int i = (int)scopes.size() - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return it->second;
    }
    std::cerr << "Error: undeclared variable '" << name << "'\n";
    exit(1);
}

void SymbolTable::enterScope() { scopes.push_back({}); }

void SymbolTable::leaveScope() {
    if (scopes.size() <= 1) {
        std::cerr << "Internal error: cannot pop global scope\n";
        exit(1);
    }
    scopes.pop_back();
}

int SymbolTable::depth() const { return (int)scopes.size() - 1; }

void SymbolTable::insertFunc(const std::string& name, const FuncSignature& sig) {
    if (funcs.count(name)) {
        // Instead of error, validate signature
        if (funcs[name].paramTypes != sig.paramTypes ||
            funcs[name].returnType != sig.returnType) {
            std::cerr << "Error: conflicting declaration of function '" << name << "'\n";
            exit(1);
        }
        return; // ✅ ignore duplicate
    }
    funcs[name] = sig;
}

bool SymbolTable::funcExists(const std::string& name) const {
    return funcs.count(name) > 0;
}

FuncSignature SymbolTable::getFunc(const std::string& name) const {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
        std::cerr << "Error: undefined function '" << name << "'\n";
        exit(1);
    }
    return it->second;
}

void SymbolTable::print() const {
    std::cout << "\n=== Symbol Table ===\n";
    for (int i = 0; i < (int)scopes.size(); ++i) {
        std::cout << "  [scope " << i << (i == 0 ? " -- global" : "") << "]\n";
        for (auto& [name, s] : scopes[i]) {
            std::cout << "    " << name << " : " << s.type
                      << " (" << irTypeName(s.irType) << ")";
            if (s.isArray) {
                std::cout << "[";
                for (size_t d = 0; d < s.dimensions.size(); ++d) {
                    if (d > 0) std::cout << "][";
                    std::cout << s.dimensions[d];
                }
                std::cout << "]";
            }
            std::cout << "\n";
        }
    }
    if (!funcs.empty()) {
        std::cout << "  [functions]\n";
        for (auto& [name, sig] : funcs) {
            std::cout << "    " << irTypeName(sig.returnType) << " " << name << "(";
            for (int i = 0; i < (int)sig.paramNames.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << irTypeName(sig.paramTypes[i]) << " " << sig.paramNames[i];
            }
            std::cout << ")\n";
        }
    }
}