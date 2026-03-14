#include "codegen.h"
#include "ir.h"
#include <fstream>
#include <cctype>  // for isdigit

// Helper: convert IR operand to assembly operand
std::string getOperand(const std::string& irOp, const RegisterAllocation& alloc) {
    if (alloc.varToReg.count(irOp)) {
        return regNames[alloc.varToReg.at(irOp)];
    } else if (alloc.varToStackOffset.count(irOp)) {
        return "[rbp - " + std::to_string(alloc.varToStackOffset.at(irOp)) + "]";
    } else {
        // Constants or labels
        return irOp;
    }
}

void generateAssembly(const std::string& filename, const RegisterAllocation& alloc) {
    std::ofstream out(filename);
    
    // Assembly header
    out << "global main\n";
    out << "section .text\n";
    out << "main:\n";
    out << "    push rbp\n";
    out << "    mov rbp, rsp\n";
    if (alloc.stackSize > 0) {
        out << "    sub rsp, " << alloc.stackSize << "\n";
    }
    
    // Generate code for each IR instruction
    for (const auto& instr : ir) {
        if (instr.op == "=" && instr.arg2.empty()) {
            // Simple assignment: result = arg1
            std::string dest = getOperand(instr.result, alloc);
            std::string src = getOperand(instr.arg1, alloc);
            out << "    mov " << dest << ", " << src << "\n";
        } else if (instr.op == "+") {
            // Binary op: result = arg1 + arg2
            std::string dest = getOperand(instr.result, alloc);
            std::string op1 = getOperand(instr.arg1, alloc);
            std::string op2 = getOperand(instr.arg2, alloc);
            out << "    mov " << dest << ", " << op1 << "\n";
            out << "    add " << dest << ", " << op2 << "\n";
        } else if (instr.op == "-") {
            std::string dest = getOperand(instr.result, alloc);
            std::string op1 = getOperand(instr.arg1, alloc);
            std::string op2 = getOperand(instr.arg2, alloc);
            out << "    mov " << dest << ", " << op1 << "\n";
            out << "    sub " << dest << ", " << op2 << "\n";
        } else if (instr.op == "*") {
            std::string dest = getOperand(instr.result, alloc);
            std::string op1 = getOperand(instr.arg1, alloc);
            std::string op2 = getOperand(instr.arg2, alloc);
            out << "    mov " << dest << ", " << op1 << "\n";
            out << "    imul " << dest << ", " << op2 << "\n";
        } else if (instr.op == "/") {
            // Division is more complex, need to handle
            out << "    ; division not implemented yet\n";
        } else if (instr.op == "label") {
            out << instr.arg1 << ":\n";
        } else if (instr.op == "goto") {
            out << "    jmp " << instr.arg1 << "\n";
        } else if (instr.op == "ifzero_goto") {
            std::string cond = getOperand(instr.arg1, alloc);
            out << "    test " << cond << ", " << cond << "\n";
            out << "    jz " << instr.arg2 << "\n";
        } else if (instr.op == "comment") {
            out << "    ; " << instr.arg1 << "\n";
        }
        // Add more cases as needed
    }
    
    // Footer
    out << "    mov rax, 0\n";  // return 0
    if (alloc.stackSize > 0) {
        out << "    add rsp, " << alloc.stackSize << "\n";
    }
    out << "    pop rbp\n";
    out << "    ret\n";
}