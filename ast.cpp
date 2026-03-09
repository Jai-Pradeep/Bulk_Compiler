#include "ast.h"
#include "ir.h"
#include "symtab.h"
#include "depcheck.h"
#include <iostream>

static void indentPrint(int n) { for (int i = 0; i < n; i++) std::cout << "  "; }

// ─── NumberNode ───────────────────────────────────────────────────────────────
NumberNode::NumberNode(int v) : value(v) { irType = IRType::INT32; }
void NumberNode::print(int indent) { indentPrint(indent); std::cout << "Number " << value << "\n"; }
std::string NumberNode::generateIR() { irType = IRType::INT32; return std::to_string(value); }

// ─── IdentifierNode ───────────────────────────────────────────────────────────
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

// ─── BinaryOpNode ─────────────────────────────────────────────────────────────
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

// ─── AssignmentNode ───────────────────────────────────────────────────────────
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

        std::string iVar, Lstart, Lend;
        emitParallelLoopHeader(n, iVar, Lstart, Lend);

        std::string lv = emitCastIfNeeded(leftId->name +"["+iVar+"]", a.irType, resT);
        std::string rv = emitCastIfNeeded(rightId->name+"["+iVar+"]", b.irType, resT);
        std::string t  = newTemp();
        ir.emplace_back(bin->op, lv, rv, t, resT);
        ir.emplace_back("=", emitCastIfNeeded(t, resT, lhs.irType), "", name+"["+iVar+"]", lhs.irType);

        emitParallelLoopFooter(iVar, Lstart, Lend);
        return name;
    }
    if (leftId && rightNum) {
        Symbol a = symtab.get(leftId->name);
        if (!a.isArray || a.size != n) { std::cerr << "Error: array mismatch\n"; exit(1); }
        IRType resT = dominantType(a.irType, IRType::INT32);

        std::string iVar, Lstart, Lend;
        emitParallelLoopHeader(n, iVar, Lstart, Lend);

        std::string lv = emitCastIfNeeded(leftId->name+"["+iVar+"]", a.irType, resT);
        std::string t  = newTemp();
        ir.emplace_back(bin->op, lv, std::to_string(rightNum->value), t, resT);
        ir.emplace_back("=", emitCastIfNeeded(t, resT, lhs.irType), "", name+"["+iVar+"]", lhs.irType);

        emitParallelLoopFooter(iVar, Lstart, Lend);
        return name;
    }
    if (leftId && rightElem) {
        Symbol a = symtab.get(leftId->name), es = symtab.get(rightElem->name);
        if (!a.isArray || a.size != n) { std::cerr << "Error: array mismatch\n"; exit(1); }
        IRType resT = dominantType(a.irType, es.irType);

        std::string iVar, Lstart, Lend;
        emitParallelLoopHeader(n, iVar, Lstart, Lend);

        // rightElem index needs the loop var substituted
        std::string elem = rightElem->name + "[" + rightElem->index->generateIR() + "]";
        std::string lv   = emitCastIfNeeded(leftId->name+"["+iVar+"]", a.irType,  resT);
        elem             = emitCastIfNeeded(elem,                       es.irType, resT);
        std::string t    = newTemp();
        ir.emplace_back(bin->op, lv, elem, t, resT);
        ir.emplace_back("=", emitCastIfNeeded(t, resT, lhs.irType), "", name+"["+iVar+"]", lhs.irType);

        emitParallelLoopFooter(iVar, Lstart, Lend);
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

// ─── ComparisonNode ───────────────────────────────────────────────────────────
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

// ─── StatementListNode ────────────────────────────────────────────────────────
void StatementListNode::print(int indent) {
    for (auto* s : stmts) if (s) s->print(indent);
}
std::string StatementListNode::generateIR() {
    for (auto* s : stmts) if (s) s->generateIR();
    return "";
}

// ─── ForNode ─────────────────────────────────────────────────────────────────
//
//  Correct three-address IR layout:
//
//      <init>            ; i = 0
//  Lstart:
//      <cond>            ; t = i < 5
//      if t == 0 goto Lend
//      <body>            ; a[i] = i * 2
//      <step>            ; i = i + 1
//      goto Lstart
//  Lend:
//
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

    // ── Dependency analysis before emitting any IR ────────────────────────────
    // Extract the loop index variable name from the init node (AssignmentNode)
    std::string loopVar = "i";  // fallback
    if (AssignmentNode* a = dynamic_cast<AssignmentNode*>(init))
        loopVar = a->name;

    DepReport dep = analyzeLoop(body, loopVar);

    // ── Emit parallelism annotation as IR comment ─────────────────────────────
    switch (dep.kind) {
        case ParallelKind::FULLY_PARALLEL:
            ir.emplace_back("comment",
                "PARALLEL LOOP — fully parallel, all iterations independent"
                " [SIMD/multi-thread safe]", "", "");
            for (auto& arr : dep.parallelArrays)
                ir.emplace_back("comment",
                    "  parallel array: " + arr + "[" + loopVar + "]", "", "");
            break;

        case ParallelKind::PARTIAL_PARALLEL:
            ir.emplace_back("comment",
                "PARTIALLY PARALLEL LOOP — some iterations can run in parallel", "", "");
            for (auto& arr : dep.parallelArrays)
                ir.emplace_back("comment",
                    "  safe to parallelise: " + arr + "[" + loopVar + "]", "", "");
            for (auto& [arr, reason] : dep.blockers)
                ir.emplace_back("comment",
                    "  DEPENDENCY on " + arr + ": " + reason, "", "");
            break;

        case ParallelKind::NOT_PARALLEL:
            ir.emplace_back("comment",
                "SEQUENTIAL LOOP — loop-carried dependencies prevent parallelism", "", "");
            for (auto& [arr, reason] : dep.blockers)
                ir.emplace_back("comment",
                    "  DEPENDENCY: " + reason, "", "");
            break;

        case ParallelKind::UNKNOWN:
            ir.emplace_back("comment",
                "LOOP — no array accesses detected, parallelism unknown", "", "");
            break;
    }

    // ── Emit actual loop IR ───────────────────────────────────────────────────
    std::string Lstart = newLabel();
    std::string Lend   = newLabel();

    // 1. Init
    if (init) init->generateIR();

    // 2. Loop header label
    emitLabel(Lstart);

    // 3. Condition
    std::string condTemp = cond ? cond->generateIR() : "1";

    // 4. Exit branch
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

// ─── IfNode ───────────────────────────────────────────────────────────────────
//
//  Without else:
//      <cond>
//      if cond == 0 goto Ldone
//      <thenBody>
//  Ldone:
//
//  With else:
//      <cond>
//      if cond == 0 goto Lelse
//      <thenBody>
//      goto Ldone
//  Lelse:
//      <elseBody>
//  Ldone:
//
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