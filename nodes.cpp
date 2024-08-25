#include "nodes.hpp"
#include "symTable.hpp"
#include "hw3_output.hpp"
#include "codeGen.hpp"
#include "cg.hpp"

using namespace std;

extern CodeBuffer buffer;
extern codeGenerator code_gen;
extern int yylineno;
extern symTableStack stacks;

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
    /// TODO: handle labels 
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
    symTableEntry* symbol = stacks.getSymbol(node->val);
    Exp* tempExp = new Exp();
    tempExp->reg = codeGenerator.allocateReg(0);
    std::string regAddress = codeGenerator.allocateReg(0);
    buffer.emit(tempExp->reg + " = add i32 0, 0");
    buffer.emit(regAddress + " = getelementptr i32, i32* " + stacks.rbp + ", i32 " + std::to_string(symbol->offset));
    buffer.emit("store i32 " + tempExp->reg + ", i32* " + regAddress);
    delete tempExp;
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
    symTableEntry *symbol = stacks.getSymbol(node->val);
    if (type->type == "byte") {
        string regAddress = codeGenerator.allocateReg(0);
        string regExtended = codeGenerator.allocateReg(0);
        buffer.emit(regExtended + " = zext i8 " + exp->reg + " to i32");  // Extend the 8-bit byte to a 32-bit integer
        buffer.emit(regAddress + " = getelementptr i32, i32* " + stacks.rbp + ", i32 " + std::to_string(symbol->offset));
        buffer.emit("store i32 " + regExtended + ", i32* " + regAddress); // Store in memory
        }
    else if (type->type =="int"){
        string regAddress = codeGenerator.allocateReg(0);
        string finalReg;
        if (exp->type == "byte") {
            finalReg = codeGenerator.allocateReg(0);
            buffer.emit(finalReg + " = zext i8 " + exp->reg + " to i32");
        } else {
            finalReg = exp->reg; //no need to change 
        }
        buffer.emit(regAddress + " = getelementptr i32, i32* " + stacks.rbp + ", i32 " + std::to_string(symbol->offset));
        buffer.emit("store i32 " + finalReg + ", i32* " + regAddress);
    }
    else if (type->type == "bool"){
        string regPtr = codeGenerator.allocateReg(0);
        string regExtended = codeGenerator.allocateReg(0);
        buffer.emit(regExtended + " = zext i1 " + exp->reg + " to i32");  // Extend the 1-bit to 32-bit
        buffer.emit(regPtr + " = getelementptr i32, i32* " + stacks.rbp + ", i32 " + std::to_string(symbol->offset));
        buffer.emit("store i32 " + regExtended + ", i32* " + regPtr);
    }
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
        if (symbol->params == "string")
            toPrint = "STRING";
        else if (symbol->params == "int")
            toPrint = "INT";
        else if (symbol->params == "bool")
            toPrint = "BOOL";
        else if (symbol->params == "byte")
            toPrint = "BYTE";
        output::errorPrototypeMismatch(yylineno, name, toPrint);
        exit(0);
    }

    // we found the right function!
    reg = code_gen.allocateReg(0);
    this->val = name;
    this->returnType = symbol->type;

    if (returnType == "void" && (exp->type == "int" || exp->type == "byte") && val == "printi")
    {
        string new_reg = code_gen.allocateReg(0);
        if (exp->type == "byte")
        {
            buffer.emit(new_reg + " = zext i8 " + exp->reg + " to i32");
        }
        else
        {
            buffer.emit(new_reg + " = add i32 " + exp->reg + ", 0");
        }
        buffer.emit("call void @" + val + "(i32 " + new_reg + ")");
    }
    else if (returnType == "int" && exp->type == "int" && val == "readi")
    {
        buffer.emit(reg + " = call i32 @" + val + "(i32 " + exp->reg + ")");
    }
    else if (returnType == "void" && exp->type == "string" && val == "print")
    {
        buffer.emit("call void @" + val + "(i8* " + exp->reg + ")");
    }
    
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

//defalut constructor:
Exp::Exp(): Node(), type("void"), isVar(false)
{
    reg = code_gen.allocateReg(0);
}

// Exp -> LPAREN Exp RPAREN
Exp::Exp(const Exp *other) : Node(other->val), type(other->type)
{
    reg = code_gen.allocateReg(false);
    if (type == "int")
    {
        buffer.emit(reg + " = add i32 " + other->reg + ", 0");
    }
    else if (type == "bool")
    {
        buffer.emit(reg + " = add i1 " + other->reg + ", 0");
    }
    else if (type == "string")
    {
        buffer.emit(reg + " = add i8* " + other->reg + ", 0");
    }
    else if (type == "byte")
    {
        buffer.emit(reg + " = add i8 " + other->reg + ", 0");
    }
}

// EXP -> ID
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
        if (reg == "")
        {
            reg = code_gen.allocateReg(0);
        }
        string reg_ptr = code_gen.allocateReg(0);
        buffer.emit(reg_ptr + " = getelementptr i32, i32* " + stacks.rbp + ", i32 " + std::to_string(symbol->offset));
        buffer.emit(reg + " = load i32, i32* " + reg_ptr);
        if (symbol->type == "bool")
        {
            string register_c = code_gen.allocateReg(0);
            string reg_new = code_gen.allocateReg(0);
            buffer.emit(register_c + " = add i32 " + reg + ", 0");
            buffer.emit(reg_new + " = icmp ne i32 " + register_c + ", 0");
            reg = reg_new;
        }
        else if (symbol->type == "byte")
        {
            string register_c = code_gen.allocateReg(0);
            string reg_new = code_gen.allocateReg(0);
            buffer.emit(register_c + " = add i32 " + reg + ", 0");
            buffer.emit(reg_new + " = trunc i32 " + register_c + " to i8");
            reg = reg_new;
        }
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
        if (reg == "")
        {
            reg = code_gen.allocateReg(0);
        }
        reg = call->reg;
    }
}

// EXP -> NUM, STRING, TRUE, FALSE !!! not used anymore !!!
Exp::Exp(std::string type) : type(type)
{
}

// Exp -> NUM B / NUM, STRING, TRUE, FALSE
Exp::Exp(std::string type, const Node *node) : type(type), Node(node->val)
{
    if (type == "byte")
    {
        try
        {
            if (type == "byte" && node && stoi(node->val) <= 255)
            {
                this->type = type;
                reg = code_gen.allocateReg(0);
                buffer.emit(reg + " = add i8 " + val + ", 0");
                return;
            }
        }
        catch (std::out_of_range)
        {
        }
        output::errorByteTooLarge(yylineno, node->val);
        exit(0);
    }
    else if (type == "int")
    {
        this->type = type;
        reg = code_gen.allocateReg(0);
        buffer.emit(reg + " = add i32 " + val + ", 0");
    }
    else if (type == "bool")
    {
        this->type = type;
        reg = code_gen.allocateReg(0);
        if (node->val == "true")
        {
            buffer.emit(reg + " = add i1 1, 0");
        }
        else if (node->val == "false")
        {
            buffer.emit(reg + " = add i1 0, 0");
        }
    }
    else // type == string
    {
        this->type = type;
        // Extract the terminal value and remove the last character.std::string str = terminal->value;
        std::string str = node->val;
        str.pop_back();

        // Generate a fresh global variable for the constant string representation.
        std::string temp_register = code_gen.allocateReg(1);
        std::string str_length = std::to_string(str.length());

        // Emit the global constant with the adjusted string.
        buffer.emitGlobal(temp_register + " = constant [" + str_length + " x i8] c\"" + str + "\\00\"");

        // Create a temporary register pointer for the GEP (getelementptr) instruction.
        std::string temp_reg_ptr = "%" + temp_register.substr(1);
        buffer.emit(temp_reg_ptr + ".ptr = getelementptr [" + str_length + " x i8], [" + str_length + " x i8]* " + temp_register + ", i32 0, i32 0");

        // Assign the result to the output register.
        reg = temp_reg_ptr + ".ptr";
    }
}

// EXP -> NOT EXP
Exp::Exp(const Exp *operand, std::string opType)
{
    if (operand && operand->type == "bool" && opType == "logic")
    {
        this->type = "bool";
        reg = code_gen.allocateReg(0);
        true_label = buffer.freshLabel();
        false_label = buffer.freshLabel();
        next_label = buffer.freshLabel();
        buffer.emit(reg + " = sub i1 1, " + operand->reg);
    }
    else
    {
        /* TODO: Handle Error*/
    }
}

// EXP -> EXP OP EXP
Exp::Exp(const Exp *operand1, const Exp *operand2, std::string opType, std::string op)
{
    // dynamic cast to exp?

    if (operand1 && operand2)
    {

        if (opType == "logic")
        {
            if (!(operand1->type == operand2->type && operand2->type == "bool"))
            {
                output::errorMismatch(yylineno);
                exit(0);
            }
            this->type = "bool";
            true_label = buffer.freshLabel();
            false_label = buffer.freshLabel();
            next_label = buffer.freshLabel();
            if (op == "and")
            {
                buffer.emit(reg + " = and i1 " + operand1->reg + ", " + operand2->reg);
            }
            else if (op == "or")
            {
                buffer.emit(reg + " = or i1 " + operand1->reg + ", " + operand2->reg);
            }
        }
        else if (opType == "relop")
        {
            if (operand1->isNumExp() && operand2->isNumExp())
            {
                this->type = "bool";
                string fresh_reg1 = codeGenerator.freshVar();
                string fresh_reg2 = codeGenerator.freshVar();
                if (operand1->type == "int")
                {
                    buffer.emit(fresh_reg1 + " = add i32 " + operand1->reg + ", 0");
                }
                else if (operand1->type == "byte")
                {
                    buffer.emit(fresh_reg1 + " = zext i8 " + operand1->reg + " to i32");
                }

                if (operand2->type == "int")
                {
                    buffer.emit(fresh_reg2 + " = add i32 " + operand2->reg + ", 0");
                }
                else if (operand2->type == "byte")
                {
                    buffer.emit(fresh_reg2 + " = zext i8 " + operand2->reg + " to i32");
                }

                string op_text = code_gen.relopGetter(op);
                buffer.emit(reg + "= icmp " + op_text + " i32 " + fresh_reg1 + ", " + fresh_reg2);
            }
            output::errorMismatch(yylineno);
            exit(0);
        }
        else if (opType == "arithmetic")
        {
            // handling an unrelated type
            if ((operand1->type != "int" && operand1->type != "byte") || (operand2->type != "int" && operand2->type != "byte"))
            {
                output::errorMismatch(yylineno);
                exit(0);
            }

            if (operand1->isNumExp() && operand2->isNumExp())
            {
                if (operand1->type == "int" || operand2->type == "int")
                {
                    this->type = "int";
                    string fresh_reg1 = code_gen.allocateReg();
                    string fresh_reg2 = code_gen.allocateReg();
                    if (operand1->type == "int")
                    {
                        buffer.emit(fresh_reg1 + " = add i32 " + operand1->reg + ", 0");
                    }
                    else if (operand1->type == "byte")
                    {
                        buffer.emit(fresh_reg1 + " = zext i8 " + operand1->reg + " to i32");
                    }
                    if (operand2->type == "int")
                    {
                        buffer.emit(fresh_reg2 + " = add i32 " + operand2->reg + ", 0");
                    }
                    else if (operand2->type == "byte")
                    {
                        buffer.emit(fresh_reg2 + " = zext i8 " + operand2->reg + " to i32");
                    }

                    string text_op = binopGetter(op);
                    if (text_op == "div")
                    {
                        buffer.emit("call void @check_division(i32 " + fresh_reg2 + ")");
                        buffer.emit(reg + " = sdiv i32 " + fresh_reg1 + ", " + fresh_reg2);
                    }
                    else
                    {
                        buffer.emit(reg + " = " + text_op + " i32 " + fresh_reg1 + ", " + fresh_reg2);
                    }
                }
                else
                {
                    this->type = "byte";
                    string text_op = binopGetter(op);
                    if (text_op == "div")
                    {
                        string old_reg = code_gen.freshVar();
                        buffer.emit(old_reg + " = zext i8 " + operand2->reg + "to i32");
                        buffer.emit("call void @check_division(i32 " + old_reg + ")");
                        buffer.emit(reg + " = udiv i8 " + operand1->reg + ", " + operand2->reg);
                    }
                    else
                    {
                        buffer.emit(reg + " = " + text_op + " i8 " + operand1->reg + ", " + operand2->reg);
                    }
                }
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
        reg = code_gen.allocateReg(0);

        if (type->type == "int" && operand->type == "int")
        {
            buffer.emit(reg + " = add i32 " + operand->reg + ", 0");
            this->type = "int";
        }
        else if (type->type == "byte" && operand->type == "int")
        {
            buffer.emit(reg + " = trunc i32 " + operand->reg + " to i8");
            this->type = "byte";
        }
        else if (type->type == "int" && operand->type == "byte")
        {
            buffer.emit(reg + " = zext i8 " + operand->reg + " to i32");
            this->type = "int";
        }
        else if (type->type == "byte" && operand->type == "byte")
        {
            buffer.emit(reg + " = add i8 " + operand->reg + ", 0");
            this->type = "byte";
        }
    }
    else if (type->type != "byte" || type->type != "int")
    {
        output::errorMismatch(yylineno);
        exit(0);
    }
    else if (operand->type != "byte" || operand->type != "int")
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

Label::Label() : Node("")
{
    true_label = buffer.freshLabel(); 
    false_label = buffer.freshLabel(); 
    next_label = buffer.freshLabel(); 
}

ifClass::ifClass(Exp* exp, Label* label)
{
    this->exp = exp;
    this->label = label;
}

// -----