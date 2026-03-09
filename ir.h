#ifndef IR_H
#define IR_H

#include <string>
#include <vector>

class IRInstruction {

public:

    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;

    IRInstruction(std::string o, std::string a1, std::string a2, std::string r);

};

extern std::vector<IRInstruction> ir;

std::string newTemp();

void printIR();

void emitForLoop(std::string idx, int size);

void endForLoop();

#endif