#pragma once
#include <string>
#include <vector>
#include "ir.h"

// ── Base node ─────────────────────────────────────────────────────────────────
struct ASTNode {
    IRType irType = IRType::UNKNOWN;
    virtual void        print      (int indent = 0) = 0;
    virtual std::string generateIR ()               = 0;
    virtual ~ASTNode() {}
};

// ── Existing nodes ────────────────────────────────────────────────────────────
struct NumberNode : ASTNode {
    int value;
    NumberNode(int v);
    void print(int indent) override;
    std::string generateIR() override;
};

struct FloatNode : ASTNode {
    double value;
    FloatNode(double v);
    void print(int indent) override;
    std::string generateIR() override;
};

struct CharNode : ASTNode {
    char value;
    CharNode(char v);
    void print(int indent) override;
    std::string generateIR() override;
};

struct BoolNode : ASTNode {
    bool value;
    BoolNode(bool v);
    void print(int indent) override;
    std::string generateIR() override;
};

struct IdentifierNode : ASTNode {
    std::string name;
    IdentifierNode(std::string n);
    void print(int indent) override;
    std::string generateIR() override;
};

struct ArrayAccessNode : ASTNode {
    std::string name;
    std::vector<ASTNode*> indices;
    ArrayAccessNode(std::string n, std::vector<ASTNode*> i);
    void print(int indent) override;
    std::string generateIR() override;
};

struct BinaryOpNode : ASTNode {
    std::string op;
    ASTNode*    left;
    ASTNode*    right;
    BinaryOpNode(std::string o, ASTNode* l, ASTNode* r);
    void print(int indent) override;
    std::string generateIR() override;
};

struct UnaryOpNode : ASTNode {
    std::string op;
    ASTNode*    expr;
    UnaryOpNode(std::string o, ASTNode* e);
    void print(int indent) override;
    std::string generateIR() override;
};

struct AssignmentNode : ASTNode {
    std::string name;
    ASTNode*    expr;
    AssignmentNode(std::string n, ASTNode* e);
    void print(int indent) override;
    std::string generateIR() override;
};

struct ArrayElementAssignmentNode : ASTNode {
    std::string name;
    std::vector<ASTNode*> indices;
    ASTNode*    expr;
    ArrayElementAssignmentNode(std::string n, std::vector<ASTNode*> i, ASTNode* e);
    void print(int indent) override;
    std::string generateIR() override;
};

struct ComparisonNode : ASTNode {
    std::string op;
    ASTNode*    left;
    ASTNode*    right;
    ComparisonNode(std::string o, ASTNode* l, ASTNode* r);
    void print(int indent) override;
    std::string generateIR() override;
};

struct StatementListNode : ASTNode {
    std::vector<ASTNode*> stmts;
    void add(ASTNode* s) { if (s) stmts.push_back(s); }
    void print(int indent) override;
    std::string generateIR() override;
};

struct ForNode : ASTNode {
    ASTNode* init;
    ASTNode* cond;
    ASTNode* step;
    ASTNode* body;
    ForNode(ASTNode* ini, ASTNode* cnd, ASTNode* stp, ASTNode* bdy);
    void print(int indent) override;
    std::string generateIR() override;
};

struct IfNode : ASTNode {
    ASTNode* condition;
    ASTNode* thenBody;
    ASTNode* elseBody;
    IfNode(ASTNode* cond, ASTNode* thenB, ASTNode* elseB);
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: While loop ──────────────────────────────────────────────────────────
struct WhileNode : ASTNode {
    ASTNode* cond;
    ASTNode* body;
    WhileNode(ASTNode* c, ASTNode* b);
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: Logical operators (&& and ||) ───────────────────────────────────────
//
//  Short-circuit evaluation:
//
//  a && b:
//    t1 = eval(a)
//    if t1 == 0 goto Lfalse   ← short-circuit: if a is false, skip b
//    t2 = eval(b)
//    t1 = t2
//  Lfalse:
//    result = t1
//
//  a || b:
//    t1 = eval(a)
//    if t1 != 0 goto Ltrue    ← short-circuit: if a is true, skip b
//    t2 = eval(b)
//    t1 = t2
//  Ltrue:
//    result = t1
//
struct LogicalOpNode : ASTNode {
    std::string op;   // "&&" or "||"
    ASTNode*    left;
    ASTNode*    right;
    LogicalOpNode(std::string o, ASTNode* l, ASTNode* r);
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: Logical not (!) ─────────────────────────────────────────────────────
//
//  !expr:
//    t1 = eval(expr)
//    t2 = t1 == 0     ← flip: 0 becomes 1, nonzero becomes 0
//
struct LogicalNotNode : ASTNode {
    ASTNode* expr;
    LogicalNotNode(ASTNode* e);
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: Function nodes ───────────────────────────────────────────────────────

// One parameter:  int32 x
struct ParamNode {
    std::string typeName;   // "int32" / "int64" / "int128"
    std::string name;
};

// Function definition:
//   int32 add(int32 x, int32 y) { ... }
//
//   IR emitted:
//     func_begin add [i32]
//       param x [i32]
//       param y [i32]
//       <body>
//       return t_last [i32]   ← from ReturnNode inside body
//     func_end add
//
struct FunctionDefNode : ASTNode {
    std::string            returnTypeName;
    std::string            name;
    std::vector<ParamNode> params;
    StatementListNode*     body;

    FunctionDefNode(std::string retType,
                    std::string n,
                    std::vector<ParamNode> p,
                    StatementListNode* b);
    void print(int indent) override;
    std::string generateIR() override;
};

// Function call:   add(a, b)
//   Can appear as an expression (right-hand side) or as a standalone statement.
//
//   IR emitted:
//     push_arg a [i32]
//     push_arg b [i32]
//     call add -> t7 [i32]
//
struct FunctionCallNode : ASTNode {
    std::string             name;
    std::vector<ASTNode*>   args;

    FunctionCallNode(std::string n, std::vector<ASTNode*> a);
    void print(int indent) override;
    std::string generateIR() override;
};

// Return statement:   return expr;
//   IR emitted:
//     return t5 [i32]
//
struct ReturnNode : ASTNode {
    ASTNode* expr;   // nullptr for void return

    ReturnNode(ASTNode* e);
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: I/O nodes ────────────────────────────────────────────────────────────
struct ScanNode : ASTNode {
    std::string varName;
    ScanNode(std::string n);
    void print(int indent) override;
    std::string generateIR() override;
};

struct PrintNode : ASTNode {
    ASTNode* expr;
    PrintNode(ASTNode* e);
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: Control flow nodes ───────────────────────────────────────────────────
struct BreakNode : ASTNode {
    BreakNode();
    void print(int indent) override;
    std::string generateIR() override;
};

struct ContinueNode : ASTNode {
    ContinueNode();
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: @arr → total size ───────────────────────────────
struct ArraySizeNode : ASTNode {
    std::string name;
    ArraySizeNode(std::string n);
    void print(int indent) override;
    std::string generateIR() override;
};

// ── NEW: @@arr → number of dimensions ────────────────────
struct ArrayDimNode : ASTNode {
    std::string name;
    ArrayDimNode(std::string n);
    void print(int indent) override;
    std::string generateIR() override;
};