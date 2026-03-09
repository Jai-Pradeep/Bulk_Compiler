#include "symtab.h"
#include <iostream>

SymbolTable symtab;

bool SymbolTable::exists(std::string name) {

    return table.find(name) != table.end();

}

void SymbolTable::insert(std::string name, std::string type, bool isArray, int size) {

    Symbol s;
    s.type = type;
    s.isArray = isArray;
    s.size = size;

    table[name] = s;

}

Symbol SymbolTable::get(std::string name) {

    return table[name];

}

void SymbolTable::print() {

    std::cout<<"\nSymbol Table\n";

    for(auto &p : table) {

        std::cout<<p.first<<" : "<<p.second.type;

        if(p.second.isArray)
            std::cout<<"["<<p.second.size<<"]";

        std::cout<<std::endl;

    }

}
