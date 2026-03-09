#include "ir.h"
#include <iostream>

std::vector<IRInstruction> ir;

int tempCount = 0;

IRInstruction::IRInstruction(std::string o, std::string a1, std::string a2, std::string r)
{
    op = o;
    arg1 = a1;
    arg2 = a2;
    result = r;
}

std::string newTemp()
{
    tempCount++;
    return "t" + std::to_string(tempCount);
}

void emitForLoop(std::string idx, int size)
{
    std::cout << "\nFOR " << idx << " = 0 .. " << size-1 << std::endl;
}

void endForLoop()
{
    std::cout << "END FOR\n";
}

void printIR()
{
    std::cout << "\nGenerated IR\n";

    for(auto &i : ir)
    {
        if(i.op == "=")
            std::cout << i.result << " = " << i.arg1 << std::endl;

        else
            std::cout << i.result << " = " << i.arg1 << " " << i.op << " " << i.arg2 << std::endl;
    }

    std::cout << "END FOR\n";
}