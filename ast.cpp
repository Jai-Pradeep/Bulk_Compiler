#include "ast.h"
#include "ir.h"
#include "symtab.h"
#include <iostream>

void indentPrint(int n)
{
    for(int i=0;i<n;i++)
        std::cout<<"  ";
}

/* ---------------- NUMBER NODE ---------------- */

NumberNode::NumberNode(int v)
{
    value = v;
}

void NumberNode::print(int indent)
{
    indentPrint(indent);
    std::cout<<"Number "<<value<<std::endl;
}

std::string NumberNode::generateIR()
{
    return std::to_string(value);
}

/* ---------------- IDENTIFIER NODE ---------------- */

IdentifierNode::IdentifierNode(std::string n)
{
    name = n;
}

void IdentifierNode::print(int indent)
{
    indentPrint(indent);
    std::cout<<"Identifier "<<name<<std::endl;
}

std::string IdentifierNode::generateIR()
{
    return name;
}

/* ---------------- ARRAY ACCESS NODE ---------------- */

ArrayAccessNode::ArrayAccessNode(std::string n, ASTNode* i)
{
    name = n;
    index = i;
}

void ArrayAccessNode::print(int indent)
{
    indentPrint(indent);
    std::cout<<"ArrayAccess "<<name<<std::endl;
    index->print(indent+1);
}

std::string ArrayAccessNode::generateIR()
{
    std::string idx = index->generateIR();
    return name + "[" + idx + "]";
}

/* ---------------- BINARY OP NODE ---------------- */

BinaryOpNode::BinaryOpNode(std::string o, ASTNode* l, ASTNode* r)
{
    op = o;
    left = l;
    right = r;
}

void BinaryOpNode::print(int indent)
{
    indentPrint(indent);
    std::cout<<"BinaryOp "<<op<<std::endl;

    left->print(indent+1);
    right->print(indent+1);
}

std::string BinaryOpNode::generateIR()
{
    std::string l = left->generateIR();
    std::string r = right->generateIR();

    std::string t = newTemp();

    ir.push_back(IRInstruction(op,l,r,t));

    return t;
}

/* ---------------- ASSIGNMENT NODE ---------------- */

AssignmentNode::AssignmentNode(std::string n, ASTNode* e)
{
    name = n;
    expr = e;
}

void AssignmentNode::print(int indent)
{
    indentPrint(indent);
    std::cout<<"Assignment "<<name<<std::endl;
    expr->print(indent+1);
}

std::string AssignmentNode::generateIR()
{
    Symbol lhs = symtab.get(name);

    /* ---------- SCALAR ASSIGNMENT ---------- */

    if(!lhs.isArray)
    {
        IdentifierNode* id = dynamic_cast<IdentifierNode*>(expr);

        if(id != nullptr)
        {
            Symbol rhs = symtab.get(id->name);

            if(rhs.isArray)
            {
                std::cout<<"Error: cannot assign array "<<id->name
                         <<" to scalar "<<name<<std::endl;
                exit(1);
            }
        }

        std::string t = expr->generateIR();

        ir.push_back(IRInstruction("=",t,"",name));

        return name;
    }

    /* ---------- ARRAY ASSIGNMENT ---------- */

    BinaryOpNode* bin = dynamic_cast<BinaryOpNode*>(expr);

    if(bin == nullptr)
    {
        std::cout<<"Error: array assignment must be binary operation"<<std::endl;
        exit(1);
    }

    IdentifierNode* leftId = dynamic_cast<IdentifierNode*>(bin->left);
    IdentifierNode* rightId = dynamic_cast<IdentifierNode*>(bin->right);
    NumberNode* rightNum = dynamic_cast<NumberNode*>(bin->right);
    ArrayAccessNode* rightElem = dynamic_cast<ArrayAccessNode*>(bin->right);

    int n = lhs.size;

    /* ---------- ARRAY + ARRAY ---------- */

    if(leftId && rightId)
    {
        Symbol a = symtab.get(leftId->name);
        Symbol b = symtab.get(rightId->name);

        if(!a.isArray || !b.isArray)
        {
            std::cout<<"Error: operands must be arrays"<<std::endl;
            exit(1);
        }

        if(a.size != b.size || a.size != lhs.size)
        {
            std::cout<<"Error: array size mismatch"<<std::endl;
            exit(1);
        }

        emitForLoop("i", n);

        std::string t = newTemp();

        ir.push_back(IRInstruction(
            bin->op,
            leftId->name + "[i]",
            rightId->name + "[i]",
            t));

        ir.push_back(IRInstruction(
            "=",
            t,
            "",
            name + "[i]"));

        return name;
    }

    /* ---------- ARRAY + SCALAR ---------- */

    if(leftId && rightNum)
    {
        Symbol a = symtab.get(leftId->name);

        if(!a.isArray)
        {
            std::cout<<"Error: operand must be array"<<std::endl;
            exit(1);
        }

        if(a.size != lhs.size)
        {
            std::cout<<"Error: array size mismatch"<<std::endl;
            exit(1);
        }

        emitForLoop("i", n);

        std::string t = newTemp();

        ir.push_back(IRInstruction(
            bin->op,
            leftId->name + "[i]",
            std::to_string(rightNum->value),
            t));

        ir.push_back(IRInstruction(
            "=",
            t,
            "",
            name + "[i]"));

        return name;
    }

    /* ---------- ARRAY + ARRAY ELEMENT ---------- */

    if(leftId && rightElem)
    {
        Symbol a = symtab.get(leftId->name);

        if(!a.isArray)
        {
            std::cout<<"Error: operand must be array"<<std::endl;
            exit(1);
        }

        if(a.size != lhs.size)
        {
            std::cout<<"Error: array size mismatch"<<std::endl;
            exit(1);
        }

        emitForLoop("i", n);

        std::string elem = rightElem->generateIR();
        std::string t = newTemp();

        ir.push_back(IRInstruction(
            bin->op,
            leftId->name + "[i]",
            elem,
            t));

        ir.push_back(IRInstruction(
            "=",
            t,
            "",
            name + "[i]"));

        return name;
    }

    std::cout<<"Error: unsupported array operation"<<std::endl;
    exit(1);
}