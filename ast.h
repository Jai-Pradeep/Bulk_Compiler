#pragma once
#include <string>
#include <vector>
#include "ir.h"

// ── Base AST node 
struct ASTNode {
    IRType irType = IRType::UNKNOWN;
    virtual ~ASTNode() = default;
    virtual void        print      (int indent) = 0;
    virtual std::string generateIR ()           = 0;
};

// ── Leaf: integer literal 
struct NumberNode : ASTNode {
    int value;
    explicit NumberNode(int v);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── Leaf: variable reference 
struct IdentifierNode : ASTNode {
    std::string name;
    explicit IdentifierNode(std::string n);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── Leaf: array element  arr[idx] 
struct ArrayAccessNode : ASTNode {
    std::string name;
    ASTNode*    index;
    ArrayAccessNode(std::string n, ASTNode* i);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── Binary arithmetic 
struct BinaryOpNode : ASTNode {
    std::string op;
    ASTNode*    left;
    ASTNode*    right;
    BinaryOpNode(std::string o, ASTNode* l, ASTNode* r);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── Scalar assignment  name = expr 
struct AssignmentNode : ASTNode {
    std::string name;
    ASTNode*    expr;
    AssignmentNode(std::string n, ASTNode* e);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── Array element assignment  name[idx] = expr 
struct ArrayElementAssignmentNode : ASTNode {
    std::string name;
    ASTNode*    index;
    ASTNode*    expr;
    ArrayElementAssignmentNode(std::string n, ASTNode* i, ASTNode* e);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── Comparison  a < b,  a == b, … 
struct ComparisonNode : ASTNode {
    std::string op;
    ASTNode*    left;
    ASTNode*    right;
    ComparisonNode(std::string o, ASTNode* l, ASTNode* r);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── Statement list 
// Collects statements as AST nodes WITHOUT emitting IR immediately.
// generateIR() emits them all in order when called by the owner (ForNode/IfNode).
struct StatementListNode : ASTNode {
    std::vector<ASTNode*> stmts;
    void add(ASTNode* s) { if (s) stmts.push_back(s); }
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── For loop 
// init : full init assignment (i = 0)    -- ForNode owns and controls emission
// cond : full comparison expression      -- evaluated at top of each iteration
// step : full step assignment (i = i+1)  -- evaluated at bottom of each iteration
// body : StatementListNode               -- emitted between cond check and step
struct ForNode : ASTNode {
    ASTNode* init;
    ASTNode* cond;
    ASTNode* step;
    ASTNode* body;
    ForNode(ASTNode* ini, ASTNode* cnd, ASTNode* stp, ASTNode* bdy);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};

// ── If / if-else 
struct IfNode : ASTNode {
    ASTNode* condition;
    ASTNode* thenBody;   // StatementListNode
    ASTNode* elseBody;   // StatementListNode or nullptr
    IfNode(ASTNode* cond, ASTNode* thenB, ASTNode* elseB = nullptr);
    void        print      (int indent) override;
    std::string generateIR ()           override;
};