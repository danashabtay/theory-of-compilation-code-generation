#include "codeGen.hpp"
#include "nodes.hpp"

#include <iostream>
using namespace std;

extern CodeBuffer buffer;

codeGenerator::codeGenerator() : maxNumReg(0){}

string codeGenerator::allocateReg(bool global){
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
string codeGenerator::relopGetter(const string &operation){
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

string codeGenerator::binopGetter(const string &operation) {
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
void codeGenerator::globalCode() {
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


//---------------------Helper Emits-----------------------//

void codeGenerator::emitBranchCondition(const std::string& conditionReg, const std::string& trueLabel, const std::string& falseLabel) {
    buffer.emit("br i1 " + conditionReg + ", label %" + trueLabel + ", label %" + falseLabel);
}

void codeGenerator::emitLabel(const std::string& label) {
    buffer.emit(label + ":");
}

void codeGenerator::emitUnconditionalBranch(const std::string& label) {
    buffer.emit("br label %" + label);
}

void codeGenerator::emitPhiInstruction(const std::string& resultReg, const std::string& type, const std::string& trueValue, const std::string& trueLabel, const std::string& falseValue, const std::string& falseLabel) {
    buffer.emit(resultReg + " = phi " + type + " [" + trueValue + ", %" + trueLabel + "], [" + falseValue + ", %" + falseLabel + "]");
}

void codeGenerator::emitPhiNodeForExp(const std::string& reg, const std::string& trueLabel, const std::string& falseLabel, const std::string& nextLabel) {
    emitPhiInstruction(reg, "i1", "1", trueLabel, "0", falseLabel);
    emitUnconditionalBranch(nextLabel);
    emitLabel(nextLabel);
}

void codeGenerator::emitBoolStatement(const std::string& expReg,  const std::string& rbp,  int offset){
    string regPtr = codeGenerator.allocateReg(0);
    string regExtended = codeGenerator.allocateReg(0);
    buffer.emit(regExtended + " = zext i1 " + expReg + " to i32");  // Extend the 1-bit to 32-bit
    buffer.emit(regPtr + " = getelementptr i32, i32* " + rbp + ", i32 " + std::to_string(offset));
    buffer.emit("store i32 " + regExtended + ", i32* " + regPtr);
}

void codeGenerator::emitByteStatement(const std::string& expReg,  const std::string& rbp,  int offset){
    string regAddress = codeGenerator.allocateReg(0);
    string regExtended = codeGenerator.allocateReg(0);
    buffer.emit(regExtended + " = zext i8 " + expReg + " to i32");  // Extend the 8-bit byte to a 32-bit integer
    buffer.emit(regAddress + " = getelementptr i32, i32* " + rbp + ", i32 " + std::to_string(soffset));
    buffer.emit("store i32 " + regExtended + ", i32* " + regAddress); // Store in memory
}

