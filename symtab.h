#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <unordered_map>

struct Symbol {

    std::string type;   // int / float / etc
    bool isArray;
    int size;

};

class SymbolTable {

private:

    std::unordered_map<std::string, Symbol> table;

public:

    bool exists(std::string name);

    void insert(std::string name, std::string type, bool isArray=false, int size=0);

    Symbol get(std::string name);

    void print();

};

extern SymbolTable symtab;

#endif
