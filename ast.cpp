#include "ast.h"
#include "ir.h"
#include "symtab.h"
#include <iostream>

static void indentPrint(int n) { for (int i = 0; i < n; i++) std::cout << "  "; }

// ─── NumberNode ────
NumberNode::NumberNode(int v) : value(v) { irType = IRType::INT32; }
void NumberNode::print(int indent) { indentPrint(indent); std::cout << "Number " << value << "\n"; }
std::string NumberNode::generateIR() { irType = IRType::INT32; return std::to_string(value); }

// ─── IdentifierNode 
IdentifierNode::IdentifierNode(std::string n) : name(n) {}
void IdentifierNode::print(int indent) { indentPrint(indent); std::cout << "Identifier " << name << "\n"; }
std::string IdentifierNode::generateIR() {
    if (symtab.exists(name)) irType = symtab.get(name).irType;
    return name;
}

// ─── ArrayAccessNode ──────────────────────────────────────────────────────────
ArrayAccessNode::ArrayAccessNode(std::string n, ASTNode* i) : name(n), index(i) {}
void ArrayAccessNode::print(int indent) {
    indentPrint(indent); std::cout << "ArrayAccess " << name << "\n";
    index->print(indent + 1);
}
std::string ArrayAccessNode::generateIR() {
    if (symtab.exists(name)) irType = symtab.get(name).irType;
    return name + "[" + index->generateIR() + "]";
}

// ─── BinaryOpNode ──
BinaryOpNode::BinaryOpNode(std::string o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}
void BinaryOpNode::print(int indent) {
    indentPrint(indent); std::cout << "BinaryOp " << op << "\n";
    left->print(indent+1); right->print(indent+1);
}
std::string BinaryOpNode::generateIR() {
    std::string lv = left->generateIR();
    std::string rv = right->generateIR();
    IRType lT = left->irType, rT = right->irType;
    IRType resT = dominantType(lT, rT);
    lv = emitCastIfNeeded(lv, lT, resT);
    rv = emitCastIfNeeded(rv, rT, resT);
    std::string t = newTemp();
    ir.emplace_back(op, lv, rv, t, resT);
    irType = resT;
    return t;
}

// ─── AssignmentNode 
AssignmentNode::AssignmentNode(std::string n, ASTNode* e) : name(n), expr(e) {}
void AssignmentNode::print(int indent) {
    indentPrint(indent); std::cout << "Assignment " << name << "\n";
    expr->print(indent+1);
}
std::string AssignmentNode::generateIR() {
    Symbol lhs = symtab.get(name);

    if (!lhs.isArray) {
        // Guard: bare array name used in scalar context
        auto checkNotArray = [&](ASTNode* node) {
            IdentifierNode* id = dynamic_cast<IdentifierNode*>(node);
            if (id && symtab.exists(id->name) && symtab.get(id->name).isArray) {
                std::cerr << "Error: array '" << id->name
                          << "' used in scalar expression assigned to '" << name << "'\n";
                exit(1);
            }
        };
        BinaryOpNode* bin = dynamic_cast<BinaryOpNode*>(expr);
        if (bin) { checkNotArray(bin->left); checkNotArray(bin->right); }
        checkNotArray(expr);

        std::string val = expr->generateIR();
        val = emitCastIfNeeded(val, expr->irType, lhs.irType);
        ir.emplace_back("=", val, "", name, lhs.irType);
        irType = lhs.irType;
        return name;
    }

    // Array bulk assignment (kept from before)
    BinaryOpNode* bin = dynamic_cast<BinaryOpNode*>(expr);
    if (!bin) { std::cerr << "Error: array assignment must be binary op\n"; exit(1); }

    IdentifierNode*  leftId    = dynamic_cast<IdentifierNode*> (bin->left);
    IdentifierNode*  rightId   = dynamic_cast<IdentifierNode*> (bin->right);
    NumberNode*      rightNum  = dynamic_cast<NumberNode*>     (bin->right);
    ArrayAccessNode* rightElem = dynamic_cast<ArrayAccessNode*>(bin->right);
    int n = lhs.size;

    if (leftId && rightId) {
        Symbol a = symtab.get(leftId->name), b = symtab.get(rightId->name);
        if (!a.isArray || !b.isArray) { std::cerr << "Error: operands must be arrays\n"; exit(1); }
        if (a.size != b.size || a.size != n) { std::cerr << "Error: size mismatch\n"; exit(1); }
        IRType resT = dominantType(a.irType, b.irType);
        emitForLoop("i", n);
        std::string lv = emitCastIfNeeded(leftId->name+"[i]", a.irType, resT);
        std::string rv = emitCastIfNeeded(rightId->name+"[i]", b.irType, resT);
        std::string t = newTemp();
        ir.emplace_back(bin->op, lv, rv, t, resT);
        ir.emplace_back("=", emitCastIfNeeded(t,resT,lhs.irType), "", name+"[i]", lhs.irType);
        return name;
    }
    if (leftId && rightNum) {
        Symbol a = symtab.get(leftId->name);
        if (!a.isArray || a.size != n) { std::cerr << "Error: array mismatch\n"; exit(1); }
        IRType resT = dominantType(a.irType, IRType::INT32);
        emitForLoop("i", n);
        std::string lv = emitCastIfNeeded(leftId->name+"[i]", a.irType, resT);
        std::string t = newTemp();
        ir.emplace_back(bin->op, lv, std::to_string(rightNum->value), t, resT);
        ir.emplace_back("=", emitCastIfNeeded(t,resT,lhs.irType), "", name+"[i]", lhs.irType);
        return name;
    }
    if (leftId && rightElem) {
        Symbol a = symtab.get(leftId->name), es = symtab.get(rightElem->name);
        if (!a.isArray || a.size != n) { std::cerr << "Error: array mismatch\n"; exit(1); }
        IRType resT = dominantType(a.irType, es.irType);
        emitForLoop("i", n);
        std::string elem = rightElem->generateIR();
        std::string lv = emitCastIfNeeded(leftId->name+"[i]", a.irType, resT);
        elem = emitCastIfNeeded(elem, es.irType, resT);
        std::string t = newTemp();
        ir.emplace_back(bin->op, lv, elem, t, resT);
        ir.emplace_back("=", emitCastIfNeeded(t,resT,lhs.irType), "", name+"[i]", lhs.irType);
        return name;
    }
    std::cerr << "Error: unsupported array operation\n"; exit(1);
}

// ─── ArrayElementAssignmentNode ───────────────────────────────────────────────
ArrayElementAssignmentNode::ArrayElementAssignmentNode(std::string n, ASTNode* i, ASTNode* e)
    : name(n), index(i), expr(e) {}
void ArrayElementAssignmentNode::print(int indent) {
    indentPrint(indent); std::cout << "ArrayElementAssign " << name << "[]\n";
    index->print(indent+1); expr->print(indent+1);
}
std::string ArrayElementAssignmentNode::generateIR() {
    std::string idx = index->generateIR();
    std::string val = expr->generateIR();
    Symbol lhs = symtab.get(name);
    val = emitCastIfNeeded(val, expr->irType, lhs.irType);
    ir.emplace_back("=", val, "", name+"["+idx+"]", lhs.irType);
    irType = lhs.irType;
    return name+"["+idx+"]";
}

// ─── ComparisonNode 
ComparisonNode::ComparisonNode(std::string o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}
void ComparisonNode::print(int indent) {
    indentPrint(indent); std::cout << "Comparison " << op << "\n";
    left->print(indent+1); right->print(indent+1);
}
std::string ComparisonNode::generateIR() {
    std::string lv = left->generateIR(), rv = right->generateIR();
    IRType lT = left->irType, rT = right->irType;
    IRType resT = dominantType(lT, rT);
    lv = emitCastIfNeeded(lv, lT, resT);
    rv = emitCastIfNeeded(rv, rT, resT);
    std::string t = newTemp();
    ir.emplace_back(op, lv, rv, t, resT);
    irType = resT;
    return t;
}

// ─── StatementListNode 
void StatementListNode::print(int indent) {
    for (auto* s : stmts) if (s) s->print(indent);
}
std::string StatementListNode::generateIR() {
    for (auto* s : stmts) if (s) s->generateIR();
    return "";
}

// ─── ForNode 
ForNode::ForNode(ASTNode* ini, ASTNode* cnd, ASTNode* stp, ASTNode* bdy)
    : init(ini), cond(cnd), step(stp), body(bdy) {}

void ForNode::print(int indent) {
    indentPrint(indent); std::cout << "FOR (init; cond; step)\n";
    indentPrint(indent+1); std::cout << "Init:\n";  if (init) init->print(indent+2);
    indentPrint(indent+1); std::cout << "Cond:\n";  if (cond) cond->print(indent+2);
    indentPrint(indent+1); std::cout << "Step:\n";  if (step) step->print(indent+2);
    indentPrint(indent+1); std::cout << "Body:\n";  if (body) body->print(indent+2);
}

std::string ForNode::generateIR() {
    std::string Lstart = newLabel();
    std::string Lend   = newLabel();

    // 1. Init
    if (init) init->generateIR();

    // 2. Loop header
    emitLabel(Lstart);

    // 3. Condition — emits comparison instruction, returns the result temp
    std::string condTemp = cond ? cond->generateIR() : "1";

    // 4. Exit if condition is false (0)
    emitIfZero(condTemp, Lend);

    // 5. Body
    if (body) body->generateIR();

    // 6. Step
    if (step) step->generateIR();

    // 7. Back-edge
    emitGoto(Lstart);

    // 8. Exit label
    emitLabel(Lend);

    return "";
}

// ─── IfNode 
IfNode::IfNode(ASTNode* cond, ASTNode* thenB, ASTNode* elseB)
    : condition(cond), thenBody(thenB), elseBody(elseB) {}

void IfNode::print(int indent) {
    indentPrint(indent); std::cout << "IF" << (elseBody ? " (with else)" : "") << "\n";
    indentPrint(indent+1); std::cout << "Cond:\n";
    if (condition) condition->print(indent+2);
    indentPrint(indent+1); std::cout << "Then:\n";
    if (thenBody) thenBody->print(indent+2);
    if (elseBody) {
        indentPrint(indent+1); std::cout << "Else:\n";
        elseBody->print(indent+2);
    }
}

std::string IfNode::generateIR() {
    std::string Ldone = newLabel();
    std::string Lelse = elseBody ? newLabel() : Ldone;

    std::string condTemp = condition ? condition->generateIR() : "1";
    emitIfZero(condTemp, Lelse);

    if (thenBody) thenBody->generateIR();

    if (elseBody) {
        emitGoto(Ldone);
        emitLabel(Lelse);
        elseBody->generateIR();
    }

    emitLabel(Ldone);
    return "";
}