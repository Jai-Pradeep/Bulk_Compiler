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

// ─── FloatNode ────────────────────────────────────────────────────────────────
FloatNode::FloatNode(double v) : value(v) { irType = IRType::FLOAT; }
void FloatNode::print(int indent) { indentPrint(indent); std::cout << "Float " << value << "\n"; }
std::string FloatNode::generateIR() { irType = IRType::FLOAT; return std::to_string(value); }

// ─── CharNode ─────────────────────────────────────────────────────────────────
CharNode::CharNode(char v) : value(v) { irType = IRType::CHAR; }
void CharNode::print(int indent) { indentPrint(indent); std::cout << "Char '" << value << "'\n"; }
std::string CharNode::generateIR() { irType = IRType::CHAR; return std::string(1, value); }

// ─── BoolNode ─────────────────────────────────────────────────────────────────
BoolNode::BoolNode(bool v) : value(v) { irType = IRType::BOOL; }
void BoolNode::print(int indent) { indentPrint(indent); std::cout << "Bool " << (value ? "true" : "false") << "\n"; }
std::string BoolNode::generateIR() { irType = IRType::BOOL; return value ? "1" : "0"; }

// ─── IdentifierNode ───────────────────────────────────────────────────────────
IdentifierNode::IdentifierNode(std::string n) : name(n) {}
void IdentifierNode::print(int indent) { indentPrint(indent); std::cout << "Identifier " << name << "\n"; }
std::string IdentifierNode::generateIR() {
    if (symtab.exists(name)) irType = symtab.get(name).irType;
    return name;
}

// ─── ArrayAccessNode ──────────────────────────────────────────────────────────
ArrayAccessNode::ArrayAccessNode(std::string n, std::vector<ASTNode*> i) : name(n), indices(i) {}
void ArrayAccessNode::print(int indent) {
    indentPrint(indent); std::cout << "ArrayAccess " << name << "\n";
    for (auto* idx : indices) idx->print(indent + 1);
}
std::string ArrayAccessNode::generateIR() {
    if (symtab.exists(name)) irType = symtab.get(name).irType;
    std::string res = name;
    for (auto* idx : indices) {
        res += "[" + idx->generateIR() + "]";
    }
    return res;
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

// ─── UnaryOpNode ──────────────────────────────────────────────────────────────
UnaryOpNode::UnaryOpNode(std::string o, ASTNode* e) : op(o), expr(e) {}
void UnaryOpNode::print(int indent) {
    indentPrint(indent); std::cout << "UnaryOp " << op << "\n";
    expr->print(indent+1);
}
std::string UnaryOpNode::generateIR() {
    std::string ev = expr->generateIR();
    IRType eT = expr->irType;
    std::string t = newTemp();
    if (op == "~" || op == "-" || op == "!") {
    ir.emplace_back(op, "", ev, t, eT);  // operator is prefix
} else {
    ir.emplace_back(op, ev, "", t, eT);
}
    irType = eT;
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
    int n = lhs.dimensions.empty() ? 0 : lhs.dimensions[0];

    if (leftId && rightId) {
        Symbol a = symtab.get(leftId->name), b = symtab.get(rightId->name);
        if (!a.isArray || !b.isArray) { std::cerr << "Error: operands must be arrays\n"; exit(1); }
        if (a.dimensions[0] != b.dimensions[0] || a.dimensions[0] != n) { std::cerr << "Error: size mismatch\n"; exit(1); }
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
        if (!a.isArray || a.dimensions[0] != n) { std::cerr << "Error: array mismatch\n"; exit(1); }
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
        if (!a.isArray || a.dimensions[0] != n) { std::cerr << "Error: array mismatch\n"; exit(1); }
        IRType resT = dominantType(a.irType, es.irType);

        std::string iVar, Lstart, Lend;
        emitParallelLoopHeader(n, iVar, Lstart, Lend);

        // rightElem index needs the loop var substituted
        std::string elem = rightElem->name + "[" + rightElem->indices[0]->generateIR() + "]";
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
ArrayElementAssignmentNode::ArrayElementAssignmentNode(std::string n, std::vector<ASTNode*> i, ASTNode* e)
    : name(n), indices(i), expr(e) {}
void ArrayElementAssignmentNode::print(int indent) {
    indentPrint(indent); std::cout << "ArrayElementAssign " << name << "[]\n";
    for (auto* idx : indices) idx->print(indent+1);
    expr->print(indent+1);
}
std::string ArrayElementAssignmentNode::generateIR() {
    std::string val = expr->generateIR();
    Symbol lhs = symtab.get(name);
    val = emitCastIfNeeded(val, expr->irType, lhs.irType);
    
    // Build subscript string with all indices: mat[i][j][k]...
    std::string subscript = name;
    for (auto* idx : indices) {
        subscript += "[" + idx->generateIR() + "]";
    }
    
    ir.emplace_back("=", val, "", subscript, lhs.irType);
    irType = lhs.irType;
    return subscript;
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

// ─── WhileNode ────────────────────────────────────────────────────────────────
//
//  IR layout:
//  Lstart:
//    <cond>
//    if cond == 0 goto Lend
//    <body>
//    goto Lstart
//  Lend:
//
WhileNode::WhileNode(ASTNode* c, ASTNode* b) : cond(c), body(b) {}

void WhileNode::print(int indent) {
    indentPrint(indent); std::cout << "WHILE\n";
    indentPrint(indent+1); std::cout << "Cond:\n";
    if (cond) cond->print(indent+2);
    indentPrint(indent+1); std::cout << "Body:\n";
    if (body) body->print(indent+2);
}

std::string WhileNode::generateIR() {
    std::string Lstart = newLabel();
    std::string Lend   = newLabel();

    emitLabel(Lstart);

    std::string condTemp = cond ? cond->generateIR() : "1";
    emitIfZero(condTemp, Lend);

    if (body) body->generateIR();

    emitGoto(Lstart);
    emitLabel(Lend);
    return "";
}

// ─── LogicalOpNode ────────────────────────────────────────────────────────────
LogicalOpNode::LogicalOpNode(std::string o, ASTNode* l, ASTNode* r)
    : op(o), left(l), right(r) {}

void LogicalOpNode::print(int indent) {
    indentPrint(indent); std::cout << "LogicalOp " << op << "\n";
    left->print(indent+1); right->print(indent+1);
}

std::string LogicalOpNode::generateIR() {
    std::string Lshort = newLabel();   // short-circuit target
    std::string Ldone  = newLabel();

    std::string t1 = left->generateIR();
    std::string result = newTemp();

    if (op == "&&") {
        // If left is false (0), jump straight to done — result stays 0
        ir.emplace_back("=", t1, "", result, IRType::INT32);
        emitIfZero(t1, Lshort);
        std::string t2 = right->generateIR();
        ir.emplace_back("=", t2, "", result, IRType::INT32);
        emitLabel(Lshort);
    } else {
        // || : If left is true (nonzero), jump straight to done — result stays 1
        ir.emplace_back("=", t1, "", result, IRType::INT32);
        // emit: if result != 0 goto Lshort
        // we only have ifzero, so we negate: tmp = (result == 0), if tmp goto Ldone
        std::string tnot = newTemp();
        ir.emplace_back("==", result, "0", tnot, IRType::INT32);
        emitIfZero(tnot, Lshort);   // if tnot==0 means result was nonzero → skip
        std::string t2 = right->generateIR();
        ir.emplace_back("=", t2, "", result, IRType::INT32);
        emitLabel(Lshort);
    }

    irType = IRType::INT32;
    return result;
}

// ─── LogicalNotNode ───────────────────────────────────────────────────────────
LogicalNotNode::LogicalNotNode(ASTNode* e) : expr(e) {}

void LogicalNotNode::print(int indent) {
    indentPrint(indent); std::cout << "LogicalNot\n";
    expr->print(indent+1);
}

std::string LogicalNotNode::generateIR() {
    std::string val = expr->generateIR();
    std::string t   = newTemp();
    // t = (val == 0)  →  1 if val was 0, 0 if val was nonzero
    ir.emplace_back("==", val, "0", t, IRType::INT32);
    irType = IRType::INT32;
    return t;
}

// ─── FunctionDefNode ──────────────────────────────────────────────────────────
//
//  IR layout:
//    func_begin  <name>  [retType]
//      param <p1> [type]
//      param <p2> [type]
//      <body>
//    func_end  <name>
//
FunctionDefNode::FunctionDefNode(std::string retType, std::string n,
                                 std::vector<ParamNode> p, StatementListNode* b)
    : returnTypeName(retType), name(n), params(p), body(b) {}

void FunctionDefNode::print(int indent) {
    indentPrint(indent);
    std::cout << "FuncDef " << returnTypeName << " " << name << "(";
    for (int i = 0; i < (int)params.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << params[i].typeName << " " << params[i].name;
    }
    std::cout << ")\n";
    if (body) body->print(indent + 1);
}

std::string FunctionDefNode::generateIR() {
    std::cout << "DEBUG: Function " << name
              << " param count = " << params.size() << "\n";
    IRType retType = parseType(returnTypeName);
    irType = retType;

    // Register function signature in symbol table BEFORE entering scope,
    // so recursive calls resolve correctly.
    FuncSignature sig;
    sig.returnType = retType;
    for (auto& p : params) {
        sig.paramTypes.push_back(parseType(p.typeName));
        sig.paramNames.push_back(p.name);
    }
    symtab.insertFunc(name, sig);

    // Emit function header
    emitFuncBegin(name, retType);

    // Enter a new scope for parameters + body
    symtab.enterScope();

    // Declare each parameter as a local variable in this scope
    for (auto& p : params) {
        symtab.insert(p.name, p.typeName, std::vector<int>{});
        emitParam(p.name, parseType(p.typeName));
    }

    // Emit body
    if (body) body->generateIR();

    // Leave function scope
    symtab.leaveScope();

    emitFuncEnd(name);
    return "";
}

// ─── FunctionCallNode ─────────────────────────────────────────────────────────
FunctionCallNode::FunctionCallNode(std::string n, std::vector<ASTNode*> a)
    : name(n), args(a) {}

void FunctionCallNode::print(int indent) {
    indentPrint(indent); std::cout << "FuncCall " << name << "\n";
    for (auto* a : args) if (a) a->print(indent + 1);
}

std::string FunctionCallNode::generateIR() {
    std::cout << "DEBUG: Calling " << name
              << " with " << args.size() << " args\n";
    if (!symtab.funcExists(name)) {
        std::cerr << "Error: call to undefined function '" << name << "'\n";
        exit(1);
    }
    FuncSignature sig = symtab.getFunc(name);
    std::cout << "DEBUG: Signature of " << name
          << " expects " << sig.paramTypes.size() << " params\n";

    if (args.size() != sig.paramTypes.size()) {
        std::cerr << "Error: function '" << name << "' expects "
                  << sig.paramTypes.size() << " arguments, got "
                  << args.size() << "\n";
        exit(1);
    }

    // Evaluate each argument and emit push_arg
    std::vector<std::string> argTemps;
    for (int i = 0; i < (int)args.size(); ++i) {
        std::string v = args[i]->generateIR();
        v = emitCastIfNeeded(v, args[i]->irType, sig.paramTypes[i]);
        emitPushArg(v, sig.paramTypes[i]);
        argTemps.push_back(v);
    }

    // Result temp (empty string if void)
    std::string resultTemp = "";
    if (sig.returnType != IRType::VOID) {
        resultTemp = newTemp();
    }

    ir.emplace_back("call", name, "", resultTemp, sig.returnType);
    irType = sig.returnType;
    return resultTemp;
}

// ─── ReturnNode ───────────────────────────────────────────────────────────────
ReturnNode::ReturnNode(ASTNode* e) : expr(e) {}

void ReturnNode::print(int indent) {
    indentPrint(indent); std::cout << "Return\n";
    if (expr) expr->print(indent + 1);
}

std::string ReturnNode::generateIR() {
    if (!expr) {
        emitReturn("", IRType::VOID);
        irType = IRType::VOID;
        return "";
    }
    std::string val = expr->generateIR();
    irType = expr->irType;
    emitReturn(val, irType);
    return val;
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

// ─── ScanNode ─────────────────────────────────────────────────────────────────
ScanNode::ScanNode(std::string n) : varName(n) {}
void ScanNode::print(int indent) {
    indentPrint(indent); std::cout << "Scan " << varName << "\n";
}
std::string ScanNode::generateIR() {
    ir.emplace_back("scan", varName, "", "", symtab.get(varName).irType);
    irType = IRType::VOID;
    return "";
}

// ─── PrintNode ────────────────────────────────────────────────────────────────
PrintNode::PrintNode(ASTNode* e) : expr(e) {}
void PrintNode::print(int indent) {
    indentPrint(indent); std::cout << "Print\n";
    if (expr) expr->print(indent + 1);
}
std::string PrintNode::generateIR() {
    std::string val = expr->generateIR();
    ir.emplace_back("print", val, "", "", expr->irType);
    irType = IRType::VOID;
    return "";
}

// ─── BreakNode ────────────────────────────────────────────────────────────────
BreakNode::BreakNode() {}
void BreakNode::print(int indent) {
    indentPrint(indent); std::cout << "Break\n";
}
std::string BreakNode::generateIR() {
    // For simplicity, emit a goto to a break label (needs context)
    // In loops, the loop node should handle labels
    ir.emplace_back("break", "", "", "", IRType::VOID);
    irType = IRType::VOID;
    return "";
}

// ─── ContinueNode ──────────────────────────────────────────────────────────────
ContinueNode::ContinueNode() {}
void ContinueNode::print(int indent) {
    indentPrint(indent); std::cout << "Continue\n";
}
std::string ContinueNode::generateIR() {
    ir.emplace_back("continue", "", "", "", IRType::VOID);
    irType = IRType::VOID;
    return "";
}