#include "codeGen.hpp"
#include <iostream>
using namespace std;

extern CodeBuffer buffer;

codeGen::codeGen() : maxNumReg(0){}

string codeGen::allocateReg(bool global){
    if(!global){
        string newReg = "%v" + std::to_string(maxNumReg++);
        return newReg;
    }
    else{ //check if needed?
        string newGreg = "@v" + std::to_string(maxNumReg++);
        return newGreg;
    }
}

//---------------------Getters-----------------------//
string codeGen::relopGetter(const string &operation){
    string text;
    if (operation == "==") {
        text = "eq";
    } else if (operation == "!=") {
        text = "ne";
    } else if (operation == ">") {
        text = "sgt";
    } else if (operation == ">=") {
        text = "sge";
    } else if (operation == "<") {
        text = "slt";
    } else {
        text = "sle";
    }
    return text;
}

string codeGen::binopGetter(const string &operation) {
    string text;
    if (operation == "+") {
        text = "add";
    } else if (operation == "-") {
        text = "sub";
    } else if (operation == "*") {
        text = "mul";
    } else {
        text = "div";
    }
    return text;
}

//---------------------handlers-----------------------//


/*
void codeGen::binopCode(Exp *exp, const Exp &op1, const Exp &op2, const string &op){
    exp->reg = allocateReg();
    string operation = binopGetter(op);
    if (operation == "div") {
        if(exp->type == "int"){
            operation = "sdiv";
        }
        else{
            operation = "udiv";
        }
        buffer.emit("call void @check_division(i32 " + op2.reg + ")");
    }
    buffer.emit(exp->reg + " = " + operation + " i32 " + op1.reg + ", " + op2.reg);
        //numerical handler for case of byte
        if (exp->type == "byte") {
            string old_reg = exp->reg;
            exp->reg = allocateReg();
            buffer.emit(exp->reg + " = and i32 255, " + old_reg);
        }
    }

void codeGen::relopCode(Exp *exp, const Exp *op1, const Exp *op2, const string &op)
{
    exp->reg = allocateReg();
    string operation = relopGetter(op);
    buffer.emit(exp->reg + " = icmp " + operation + " i32 " + op1->reg + ", " + op2->reg);
    //check if we need to use phi, bracnh or handle condition
}
*/
void codeGen::globalCode() {
    buffer.emit("@.DIV_BY_ZERO_ERROR = internal constant [23 x i8] c\"Error division by zero\\00\"");
    buffer.emit("define void @check_division(i32) {");
    buffer.emit("%valid = icmp eq i32 %0, 0");
    buffer.emit("br i1 %valid, label %ILLEGAL, label %LEGAL");
    buffer.emit("ILLEGAL:");
    buffer.emit("call void @print(i8* getelementptr([23 x i8], [23 x i8]* @.DIV_BY_ZERO_ERROR, i32 0, i32 0))");
    buffer.emit("call void @exit(i32 0)");
    buffer.emit("ret void");
    buffer.emit("LEGAL:");
    buffer.emit("ret void");
    buffer.emit("}");
}


