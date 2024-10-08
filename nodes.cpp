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
    symTableEntry *symbol = stacks.getSymbol(node->val);
    Exp *tempExp = new Exp();
    tempExp->reg = code_gen.allocateReg(0);
    std::string regAddress = code_gen.allocateReg(0);
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
    if (!stacks.doesSymbolExists(exp->val) && exp->is_variable)
    {
        output::errorUndef(yylineno, exp->val);
        exit(0);
    }
    else if (stacks.doesSymbolExists(exp->val) && exp->is_variable)
    {
        symTableEntry *symbol = stacks.getSymbol(exp->val);
        if (symbol->isFunc == exp->is_variable)
        {
            output::errorUndef(yylineno, exp->val);
            exit(0);
        }
    }
    // checking if types match
    if (type)
    {
        if (!check_types_compatible(type->type, exp->type))
        {
            //cout << "16" << endl;
            //buffer.emit("16");
            output::errorMismatch(yylineno);
            exit(0);
        }
        if (type->type == "byte" && exp->type == "int")
        {
            //cout << "17" << endl;
            //buffer.emit("17");
            output::errorMismatch(yylineno);
            exit(0);
        }
        stacks.insertSymbol(node->val, type->type, false);
    }
    else
    {
        if (exp->type == "string")
        {
            //cout << "18" << endl;
            //buffer.emit("18");
            output::errorMismatch(yylineno);
            exit(0);
        }
        stacks.insertSymbol(node->val, exp->type, false);
    }
    symTableEntry *symbol = stacks.getSymbol(node->val);
    if (type->type == "byte")
    {
        code_gen.emitByteStatement(exp->reg, stacks.rbp, symbol->offset);
    }
    else if (type->type == "int")
    {
        string regAddress = code_gen.allocateReg(0);
        string finalReg;
        if (exp->type == "byte")
        {
            finalReg = code_gen.allocateReg(0);
            code_gen.emitToStatement(finalReg, "zext", 8, exp->reg, 32);
        }
        else
        {
            finalReg = exp->reg; // no need to change
        }
        buffer.emit(regAddress + " = getelementptr i32, i32* " + stacks.rbp + ", i32 " + std::to_string(symbol->offset));
        buffer.emit("store i32 " + finalReg + ", i32* " + regAddress);
    }
    else if (type->type == "bool")
    {
        code_gen.emitBoolStatement(exp->reg, stacks.rbp, symbol->offset);
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
    if (!stacks.doesSymbolExists(exp->val) && exp->is_variable)
    {
        output::errorUndef(yylineno, exp->val);
        exit(0);
    }
    else if (stacks.doesSymbolExists(exp->val) && exp->is_variable)
    {
        symTableEntry *symbol = stacks.getSymbol(exp->val);
        if (symbol->isFunc == exp->is_variable)
        {
            output::errorUndef(yylineno, exp->val);
            exit(0);
        }
    }
    // types check
    symTableEntry *symbol = stacks.getSymbol(node->val);
    if (symbol->isFunc || !check_types_compatible(symbol->type, exp->type))
    {
        //cout << "19" << endl;
        //buffer.emit("19");

        output::errorMismatch(yylineno);
        exit(0);
    }
    if (symbol->type == "byte" && exp->type == "int")
    {
        //cout << "20" << endl;
        //buffer.emit("20");
        output::errorMismatch(yylineno);
        exit(0);
    }
    if (symbol->type == "byte")
    {
        code_gen.emitByteStatement(exp->reg, stacks.rbp, symbol->offset);
    }
    else if (symbol->type == "bool")
    {
        code_gen.emitBoolStatement(exp->reg, stacks.rbp, symbol->offset);
    }
    else if (symbol->type == "int")
    {
        string regAddress = code_gen.allocateReg(0);
        string reg = code_gen.allocateReg(0);
        buffer.emit(regAddress + " = getelementptr i32, i32* " + stacks.rbp + ", i32 " + std::to_string(symbol->offset));
        if (exp->type == "byte")
        {
            code_gen.emitToStatement(reg, "zext", 8, exp->reg, 32);
        }
        else if (exp->type == "int")
        {
            code_gen.emitZeroAddStatement(reg, "add i32 ", exp->reg);
        }
        buffer.emit("store i32 " + reg + ", i32* " + regAddress);
    }
}

/// Statement -> Call SC
Statement::Statement(Call *call) : Node() // no need to change
{
    if (!stacks.doesSymbolExists(call->val))
    {
        // cout << "3" << endl;
        output::errorUndefFunc(yylineno, call->val);
        exit(0);
    }
    symTableEntry *symbol = stacks.getSymbol(call->val);
    if (!symbol->isFunc)
    {
        // cout << "4" << endl;
        output::errorUndefFunc(yylineno, val);
        exit(0);
    }
}

///  Statement -> IF LPAREN Exp RPAREN Statement / ELSE Statement / WHILE LPAREN Exp RPAREN  Statement
Statement::Statement(Exp *exp) : Node()
{
    // checking if condition given is of type bool
    if (exp->type != "bool")
    {
        //cout << "here" << endl;
        //buffer.emit("here");
        output::errorMismatch(yylineno);
        exit(0);
    }

    this->reg = code_gen.allocateReg(0);
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
        //cout << "22" << endl;
        //buffer.emit("22");
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
            code_gen.emitToStatement(new_reg, "zext", 8, exp->reg, 32);
        }
        else
        {
            code_gen.emitZeroAddStatement(new_reg, "add i32 ", exp->reg);
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

// defalut constructor:
Exp::Exp() : Node(), type("void"), is_variable(false)
{
    reg = code_gen.allocateReg(0);
}

// Exp -> LPAREN Exp RPAREN
Exp::Exp(const Exp *other) : Node(other->val), type(other->type)
{
    reg = code_gen.allocateReg(false);
    if (type == "int")
    {
        code_gen.emitZeroAddStatement(reg, "add i32 ", other->reg);
    }
    else if (type == "bool")
    {
        code_gen.emitZeroAddStatement(reg, "add i1 ", other->reg);
    }
    else if (type == "string")
    {
        code_gen.emitZeroAddStatement(reg, "add i8* ", other->reg);
    }
    else if (type == "byte")
    {
        code_gen.emitZeroAddStatement(reg, "add i8 ", other->reg);
    }
}

// EXP -> ID
Exp::Exp(const Node *id) : Node(), is_variable(true)
{
    if (id)
    {
        if (!stacks.doesSymbolExists(id->val))
        {
            output::errorUndef(yylineno, id->val);
            exit(0);
        }
        symTableEntry *symbol = stacks.getSymbol(id->val);
        //cout << "exp type and val: " << type << val << endl;
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
            code_gen.emitZeroAddStatement(register_c, "add i32 ", reg);
            code_gen.emitZeroAddStatement(reg_new, "icmp ne i32 ", register_c);
            reg = reg_new;
        }
        else if (symbol->type == "byte")
        {
            string register_c = code_gen.allocateReg(0);
            string reg_new = code_gen.allocateReg(0);
            code_gen.emitZeroAddStatement(register_c, "add i32 ", reg);
            code_gen.emitToStatement(reg_new, "trunc", 32, register_c, 8);
            reg = reg_new;
        }
    }
}

// EXP -> CALL
Exp::Exp(const Call *call) : Node(), is_variable(false)
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
                this->val = node->val;
                reg = code_gen.allocateReg(0);
                code_gen.emitZeroAddStatement(reg, "add i8 ", val);
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
        this->val = node->val;
        reg = code_gen.allocateReg(0);
        code_gen.emitZeroAddStatement(reg, "add i32 ", val);
    }
    else if (type == "bool")
    {
        // cout << "here2" << endl;
        // buffer.emit("here2");
        this->type = type;
        //cout << "type is : " << type << endl;
        reg = code_gen.allocateReg(0);
        if (node->val == "true")
        {
            //cout << "here3" << endl;
            
            this->val = "true";

            //cout << "type and val are: " << this->type << this->val  << endl;
            buffer.emit(reg + " = add i1 1, 0");
        }
        else if (node->val == "false")
        {
            //cout << "here4" << endl;
            this->val = "false";
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
        buffer.emitGlobal(temp_register + " = constant [" + str_length + " x i8] c" + str + "\\00\"");

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
Exp::Exp(const Exp *operand1, const Exp *operand2, std::string opType, std::string op) : is_variable(false)
{
    // dynamic cast to exp?
    this->reg = code_gen.allocateReg(0);
    if (operand1 && operand2)
    {

        if (opType == "logic")
        {
            if (!(operand1->type == operand2->type && operand2->type == "bool"))
            {
                //cout << "8" << endl;
                //buffer.emit("8");
                output::errorMismatch(yylineno);
                exit(0);
            }
            this->type = "bool";
            true_label = buffer.freshLabel();
            false_label = buffer.freshLabel();
            next_label = buffer.freshLabel();
            if (op == "and")
            {
                buffer.emit(this->reg + " = and i1 " + operand1->reg + ", " + operand2->reg);
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
                string fresh_reg1 = code_gen.allocateReg(0);
                string fresh_reg2 = code_gen.allocateReg(0);
                if (operand1->type == "int")
                {
                    code_gen.emitZeroAddStatement(fresh_reg1, "add i32 ", operand1->reg);
                }
                else if (operand1->type == "byte")
                {
                    code_gen.emitToStatement(fresh_reg1, "zext", 8, operand1->reg, 32);
                }

                if (operand2->type == "int")
                {
                    code_gen.emitZeroAddStatement(fresh_reg2, "add i32 ", operand2->reg);
                }
                else if (operand2->type == "byte")
                {
                    code_gen.emitToStatement(fresh_reg2, "zext", 8, operand2->reg, 32);
                }

                string op_text = code_gen.relopGetter(op);
                buffer.emit(reg + "= icmp " + op_text + " i32 " + fresh_reg1 + ", " + fresh_reg2);
            }
            else{
                output::errorMismatch(yylineno);
                exit(0);
            }
        }
        else if (opType == "arithmetic")
        {
            // handling an unrelated type
            if ((operand1->type != "int" && operand1->type != "byte") || (operand2->type != "int" && operand2->type != "byte"))
            {
                //cout << "10" << endl;
                //buffer.emit("10");
                output::errorMismatch(yylineno);
                exit(0);
            }

            if (operand1->isNumExp() && operand2->isNumExp())
            {
                if (operand1->type == "int" || operand2->type == "int")
                {
                    this->type = "int";
                    string fresh_reg1 = code_gen.allocateReg(0);
                    string fresh_reg2 = code_gen.allocateReg(0);
                    if (operand1->type == "int")
                    {
                        code_gen.emitZeroAddStatement(fresh_reg1, "add i32 ", operand1->reg);
                    }
                    else if (operand1->type == "byte")
                    {
                        code_gen.emitToStatement(fresh_reg1, "zext", 8, operand1->reg, 32);
                    }
                    if (operand2->type == "int")
                    {
                        code_gen.emitZeroAddStatement(fresh_reg2, "add i32 ", operand2->reg);
                    }
                    else if (operand2->type == "byte")
                    {
                        code_gen.emitToStatement(fresh_reg2, "zext", 8, operand2->reg, 32);
                    }

                    string text_op = code_gen.binopGetter(op);
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
                    string text_op = code_gen.binopGetter(op);
                    if (text_op == "div")
                    {
                        string old_reg = code_gen.allocateReg(0);
                        code_gen.emitToStatement(old_reg, "zext", 8, operand2->reg, 32);
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
}

// EXP -> LPAREN Type RPAREN Exp
Exp::Exp(const Exp *operand, const Type *type) : Node(operand->val), type(type->type)
{
    if (operand && operand->isNumExp() && type && type->isNum())
    {
        //this->type = type->type;
        reg = code_gen.allocateReg(0);

        if (type->type == "int" && operand->type == "int")
        {
            code_gen.emitZeroAddStatement(reg, "add i32 ", operand->reg);
            this->type = "int";
        }
        else if (type->type == "byte" && operand->type == "int")
        {
            code_gen.emitToStatement(reg, "trunc", 32, operand->reg, 8);
            this->type = "byte";
        }
        else if (type->type == "int" && operand->type == "byte")
        {
            code_gen.emitToStatement(reg, "zext", 8, operand->reg, 32);
            this->type = "int";
        }
        else if (type->type == "byte" && operand->type == "byte")
        {
            code_gen.emitZeroAddStatement(reg, "add i8 ", operand->reg);
            this->type = "byte";
        }
    }
    else if (type->type != "byte" || type->type != "int")
    {
        //cout << "12" << endl;
        //buffer.emit("12");
        output::errorMismatch(yylineno);
        exit(0);
    }
    else if (operand->type != "byte" || operand->type != "int")
    {
        //cout << "13" << endl;
        //buffer.emit("13");
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
        //cout << "val is: " << exp->val << "type is: " << exp->type << endl;
        // buffer.emit("val is: " +  exp->val + "type is: " + exp->type);
        //cout << "14" << endl;
        // buffer.emit("14");
        output::errorMismatch(yylineno);
        exit(0);
    }
}

Label::Label() : Node("")
{
    true_label = buffer.freshLabel();
    false_label = buffer.freshLabel();
    next_label = buffer.freshLabel();
    false_label2 = buffer.freshLabel();
    true_label2 = buffer.freshLabel();
}

ifClass::ifClass(Exp *exp, Label *label)
{
    this->exp = exp;
    this->label = label;
}

// -----