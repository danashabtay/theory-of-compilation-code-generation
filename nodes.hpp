#ifndef _236360_3_NODES_H
#define _236360_3_NODES_H


#include <string>
#include <vector>
#include <iostream>




class Node
{

public:
    std::string reg = "";
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
    bool is_variable;
    std::string true_label = "";
    std::string false_label = "";
    std::string next_label = "";

    // Methods:
    Exp();
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
    Statement(): Node(){}
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


class Label: public Node{
public:
    std::string true_label;
    std::string false_label;
    std::string next_label;
    std::string false_label2;
    std::string true_label2;

    Label();
    virtual ~Label() = default;
};

class ifClass: public Exp{
public:
    Exp* exp;
    Label* label;

    ifClass(Exp* exp, Label* label);
    virtual ~ifClass() = default;
};


void check_bool(Node* node);




#endif //_236360_3_NODES_H
