#include "nodes.hpp"



using namespace std;
extern codeGen generator;


static bool check_types_compatible(string type1, string type2)
{
    if (type1 == type2)
        return true;

    if (type1 == "bool" || type2 == "bool" || type1 == "string" || type2 == "string")
        return false;
    return true;
}

// Node:

Node::Node(const std::string value) : val(value) {}

Node::Node(const Node &other) : val(other.val) {}

// std::string Node::getValue() const
// {
//     return this->val;
// }

// Program:
Program::Program() {}

// Statements:
//  in hpp file

// Statement:

// Statement -> CONTINUE SC / BREAK SC
Statement::Statement(const Node *node)
{
    if (node->val == "continue")
    {
        if (!stacks.is_loop())
        {
            output::errorUnexpectedContinue(yylineno);
            exit(0);
        }
    }
    else if (node->val == "break")
    {
        if (!stacks.is_loop())
        {
            output::errorUnexpectedBreak(yylineno);
            exit(0);
        }
    }
}

// Statement -> TYPE ID SC
Statement::Statement(const Type *type, const Node *node) : Node()
{
    if (stacks.doesSymbolExists(node->val))
    {
        output::errorDef(yylineno, node->val);
        exit(0);
    }

    stacks.insertSymbol(node->val, type->type, false);
    this->val = type->val; //?? - statement and type have no val
}

/// Statement -> TYPE ID ASSIGN EXP SC
Statement::Statement(Type *type, Node *node, Exp *exp) : Node()
{
    if (stacks.doesSymbolExists(node->val))
    {
        output::errorDef(yylineno, node->val);
        exit(0);
    }
    // checking if types match
    if (type)
    {
        if (!check_types_compatible(type->type, exp->type))
        {
            output::errorMismatch(yylineno);
            exit(0);
        }
        if (type->type == "byte" && exp->type == "int")
        {
            output::errorMismatch(yylineno);
            exit(0);
        }
        stacks.insertSymbol(node->val, type->type, false);
    }
    else
    {
        if (exp->type == "string")
        {
            output::errorMismatch(yylineno);
            exit(0);
        }
        stacks.insertSymbol(node->val, exp->type, false);
    }
    /// TODO: need to check for cases of null?
}
/// Statement -> ID ASSIGN EXP SC
Statement::Statement(Node *node, Exp *exp) : Node()
{
    // check if symbol wasn't defined
    if (!stacks.doesSymbolExists(node->val))
    {
        output::errorUndef(yylineno, node->val);
        exit(0);
    }
    // types check
    symTableEntry *symbol = stacks.getSymbol(node->val);
    if (symbol->isFunc || !check_types_compatible(symbol->type, exp->type))
    {
        output::errorMismatch(yylineno);
        exit(0);
    }
    if (symbol->type == "byte" && exp->type == "int")
    {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

/// Statement -> Call SC
Statement::Statement(Call *call) : Node()
{
    if (!stacks.doesSymbolExists(call->val))
    {
        cout << "3" << endl;
        output::errorUndefFunc(yylineno, call->val);
        exit(0);
    }
    symTableEntry *symbol = stacks.getSymbol(call->val);
    if (!symbol->isFunc)
    {
        cout << "4" << endl;
        output::errorUndefFunc(yylineno, val);
        exit(0);
    }
}

///  Statement -> IF LPAREN Exp RPAREN Statement / ELSE Statement / WHILE LPAREN Exp RPAREN  Statement
Statement::Statement(Exp *exp) : Node()
{
    // checkinh if condition given is of type bool
    if (exp->type != "bool")
    {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

// Call:

Call::Call(const Node *node, const Exp *exp)
{
    std::string name = node->val;
    // cout << "name: " << name << endl;

    // look in global scope for the function:
    if (!stacks.doesSymbolExists(name))
    {
        // cout << "1" << endl;
        output::errorUndefFunc(yylineno, name);
        exit(0);
    }

    // found a matching function symbol! check if it is a function:
    symTableEntry *symbol = stacks.getSymbol(name);
    // cout << "symbol is: "<< (symbol == nullptr) << endl;
    if (!symbol->isFunc)
    {
        // cout << "2" << endl;
        output::errorUndefFunc(yylineno, name);
        exit(0);
    }
    // cout << "symbol is func" << endl;
    //  it is a function! check argument type:
    if (!(symbol->params == exp->type) && !(symbol->params == "int" && exp->type == "byte"))
    {
        std::string toPrint;
        if (symbol->params == "string") toPrint = "STRING";
        else if(symbol->params == "int") toPrint = "INT";
        else if(symbol->params == "bool") toPrint = "BOOL";
        else if(symbol->params == "byte") toPrint = "BYTE";
        output::errorPrototypeMismatch(yylineno, name, toPrint);
        exit(0);
    }

    // we found the right function!
    this->val = name;
    this->returnType = symbol->type;
}

// Type:

Type::Type(std::string type) : Node(), type(type) {}

std::string Type::getType() const
{
    return type;
}

bool Type::isNum() const
{
    return this->type == "int" || this->type == "byte";
}

// Exp:

// Exp -> LPAREN Exp RPAREN
Exp::Exp(const Exp *other) : Node(other->val), type(other->type) {}

// EXP -> ID / CALL
Exp::Exp(const Node *id) : Node()
{
    if (id)
    {
        if (!stacks.doesSymbolExists(id->val))
        {
            output::errorUndef(yylineno, id->val);
            exit(0);
        }
        symTableEntry *symbol = stacks.getSymbol(id->val);
        this->val = id->val;
        this->type = symbol->type;
    }
}

// EXP -> CALL
Exp::Exp(const Call *call) : Node()
{
    if (call)
    {
        symTableEntry *symbol = stacks.getSymbol(call->val);
        this->val = call->val;
        this->type = symbol->type;
    }
}

// EXP -> NUM, STRING, TRUE, FALSE
Exp::Exp(std::string type) : type(type) {}

// Exp -> NUM B
Exp::Exp(std::string type, const Node *node)
{
    try
    {
        if (type == "byte" && node && stoi(node->val) <= 255)
        {
            this->type = type;
            return;
        }
    }
    catch (std::out_of_range)
    {
    }
    output::errorByteTooLarge(yylineno, node->val);
    exit(0);
}

// EXP -> NOT EXP
Exp::Exp(const Exp *operand, std::string opType)
{
    if (operand && operand->type == "bool" && opType == "logic")
    {
        this->type = "bool";
    }
    else
    {
        /* TODO: Handle Error*/
    }
}

// EXP -> EXP OP EXP
Exp::Exp(const Exp *operand1, const Exp *operand2, std::string opType)
{
    if (operand1 && operand2)
    {

        if (opType == "logic")
        {
            if (operand1->type == operand2->type && operand2->type == "bool")
            {
                this->type = "bool";
                return;
            }
        }
        else if (opType == "relop")
        {
            if (operand1->isNumExp() && operand2->isNumExp())
            {
                this->type = "bool";
                return;
            }
            else if (operand1->type != operand2->type) //is needed?
            { 
                output::errorMismatch(yylineno);
                exit(0);
            }
        }
        else if (opType == "arithmetic")
        {
            if (operand1->isNumExp() && operand2->isNumExp())
            {
                if (operand1->type == "int" || operand2->type == "int")
                {
                    this->type = "int";
                }
                else
                {
                    this->type = "byte";
                }
                return;
            }
        }
    }
    output::errorMismatch(yylineno);
    exit(0);
}

// EXP -> LPAREN Type RPAREN Exp
Exp::Exp(const Exp *operand, const Type *type)
{
    if (operand && operand->isNumExp() && type && type->isNum())
    {
        this->type = type->getType();
    }
    else if (type->type != "byte" || type->type != "int")
    {
            output::errorMismatch(yylineno);
            exit(0);
    }
    else if(operand->type != "byte" || operand->type != "int")
    {
            output::errorMismatch(yylineno);
            exit(0);
    }
}

bool Exp::isNumExp() const
{
    return this->type == "int" || this->type == "byte";
}

void check_bool(Node *node)
{
    Exp *exp = dynamic_cast<Exp *>(node);
    if (exp->type != "bool")
    {
        output::errorMismatch(yylineno);
        exit(0);
    }
}
// -----