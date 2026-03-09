#pragma once
#include <string>
#include <map>
#include "ir.h"

struct Symbol {
    std::string type;           // source-level: "int32","int64","int128","int"
    IRType      irType = IRType::INT32;  // resolved IR type
    bool        isArray = false;
    int         size    = 0;
};

class SymbolTable {
public:
    bool   exists(std::string name);
    void   insert(std::string name, std::string type, bool isArray, int size);
    Symbol get   (std::string name);
    void   print ();
private:
    std::map<std::string, Symbol> table;
};

extern SymbolTable symtab;