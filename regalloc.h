#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include "ir.h"

// Target architecture: x86-64 (16 general-purpose registers)
const int NUM_REGS = 16;
extern const char* regNames[16];  // {"rax", "rbx", ..., "r15"}

struct LiveInterval {
    std::string var;
    int start, end;  // instruction indices where var is live
    int reg = -1;    // assigned register (-1 = spilled)
};

struct RegisterAllocation {
    std::map<std::string, int> varToReg;
    std::map<std::string, int> varToStackOffset;
    int stackSize = 0;
};

// Main entry point
RegisterAllocation allocateRegisters(const std::vector<IRInstruction>& ir);

// Helper: get all variables/temporaries in IR
std::set<std::string> getAllVars(const std::vector<IRInstruction>& ir);