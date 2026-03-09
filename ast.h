#ifndef AST_H
#define AST_H

#include <string>

class ASTNode {

public:

    virtual void print(int indent = 0) = 0;

    virtual std::string generateIR() = 0;

    virtual ~ASTNode() {}

};

class NumberNode : public ASTNode {

public:

    int value;

    NumberNode(int v);

    void print(int indent);

    std::string generateIR();

};

class IdentifierNode : public ASTNode {

public:

    std::string name;

    IdentifierNode(std::string n);

    void print(int indent);

    std::string generateIR();

};

class ArrayAccessNode : public ASTNode {

public:

    std::string name;
    ASTNode* index;

    ArrayAccessNode(std::string n, ASTNode* i);

    void print(int indent);

    std::string generateIR();

};

class BinaryOpNode : public ASTNode {

public:

    std::string op;

    ASTNode* left;
    ASTNode* right;

    BinaryOpNode(std::string o, ASTNode* l, ASTNode* r);

    void print(int indent);

    std::string generateIR();

};

class AssignmentNode : public ASTNode {

public:

    std::string name;

    ASTNode* expr;

    AssignmentNode(std::string n, ASTNode* e);

    void print(int indent);

    std::string generateIR();

};

#endif