#include "regalloc.h"
#include <algorithm>
#include <cctype>  // for isdigit

// Register names (x86-64: rax, rbx, rcx, rdx, rsi, rdi, r8-r15)
const char* regNames[16] = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                           "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};

// Build live intervals for linear scan (simplified: assume all vars live throughout)
std::vector<LiveInterval> buildLiveIntervals(const std::vector<IRInstruction>& ir) {
    std::set<std::string> allVars = getAllVars(ir);
    std::vector<LiveInterval> intervals;
    
    int instrCount = ir.size();
    for (const auto& var : allVars) {
        LiveInterval li;
        li.var = var;
        li.start = 0;
        li.end = instrCount - 1;
        intervals.push_back(li);
    }
    
    return intervals;
}

// Linear scan register allocation
RegisterAllocation allocateRegisters(const std::vector<IRInstruction>& ir) {
    auto intervals = buildLiveIntervals(ir);
    RegisterAllocation alloc;
    
    bool regUsed[16] = {false};
    int nextStackOffset = 0;
    
    for (auto& interval : intervals) {
        // Find free register
        int freeReg = -1;
        for (int r = 0; r < NUM_REGS; r++) {
            if (!regUsed[r]) { freeReg = r; break; }
        }
        
        if (freeReg != -1) {
            interval.reg = freeReg;
            regUsed[freeReg] = true;
            alloc.varToReg[interval.var] = freeReg;
        } else {
            // Spill to stack
            alloc.varToStackOffset[interval.var] = nextStackOffset;
            nextStackOffset += 8;  // 8 bytes per variable
        }
    }
    
    alloc.stackSize = nextStackOffset;
    return alloc;
}

// Get all variables and temporaries in IR
std::set<std::string> getAllVars(const std::vector<IRInstruction>& ir) {
    std::set<std::string> vars;
    for (const auto& instr : ir) {
        if (!instr.result.empty() && instr.result[0] != 'L' && instr.result != "comment" && !isdigit(instr.result[0])) {
            vars.insert(instr.result);
        }
        if (!instr.arg1.empty() && instr.arg1[0] != 'L' && instr.arg1 != "comment" && !isdigit(instr.arg1[0])) {
            vars.insert(instr.arg1);
        }
        if (!instr.arg2.empty() && instr.arg2[0] != 'L' && instr.arg2 != "comment" && !isdigit(instr.arg2[0])) {
            vars.insert(instr.arg2);
        }
    }
    return vars;
}