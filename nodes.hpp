#ifndef _236360_3_NODES_H
#define _236360_3_NODES_H

#include "symTable.hpp"
#include "hw3_output.hpp"
#include "codeGen.hpp"
#include "cg.hpp"
#include <string>
#include <vector>
#include <iostream>

extern codeBuffer buffer;
extern codeGen code_gen;
extern int yylineno;
extern symTableStack stacks;


class Node
{

public:
    std::string val;

    Node(const std::string value = "");
    Node(const Node &other);
    virtual ~Node() = default;
    //std::string getValue() const;
    // Node(const Node &node): val(node.val){};
};

#define YYSTYPE Node*

class Program : public Node
{
public:
    Program();
    virtual ~Program() = default;
};

class Type : public Node
{

public:
    std::string type;

    Type(const std::string type);
    std::string getType() const;
    bool isNum() const;
    virtual ~Type() = default;
};

class Type;
class Call;

class Exp : public Node
{

public:
    std::string type;
    std::string val;
    std::string reg = "";

    // Methods:
    Exp(const Exp *other);
    Exp(const Node *id);
    Exp(const Call *call);
    Exp(std::string type);
    Exp(std::string type, const Node *node);
    Exp(const Exp *operand, std::string opType);
    Exp(const Exp *operand1, const Exp *operand2, std::string opType, std::string op);
    Exp(const Exp *operand, const Type *type);
    virtual ~Exp() = default;
    bool isNumExp() const;
};


class Call : public Node
{
public:
    std::string returnType;
    std::string reg = "";
    Call(const Node *node, const Exp *exp);
    virtual ~Call() = default;
};

class Statement : public Node
{
public:
    Statement(const Node *node);
    Statement(const Type *type, const Node *node);
    Statement(Type *type, Node *node, Exp *exp);
    Statement(Node *node, Exp *exp);
    Statement(Call *call);
    Statement(Exp *exp);
    virtual ~Statement() = default;
};

class Statements : public Node
{
public:
    Statements(Statement *statement) : Node() {};
    Statements(Statements *statements, Statement *statement) : Node() {};
    virtual ~Statements() = default;
};


void check_bool(Node* node);




#endif //_236360_3_NODES_H
