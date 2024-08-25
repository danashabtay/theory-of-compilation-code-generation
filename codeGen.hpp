#ifndef EX5_CODE_GENERATOR
#define EX5_CODE_GENERATOR

#include <string>
#include "cg.hpp"

using namespace std;


class codeGenerator{
    int maxNumReg;

public:
    codeGenerator():maxNumReg(0){}
    string allocateReg(bool global);
    string relopGetter(const string &operation);
    string binopGetter(const string &operation);
    void globalCode();
    void binopCode(Exp *exp, const Exp &op1, const Exp &op2, const string &op);
    void relopCode(Exp *exp, const Exp *op1, const Exp *op2, const string &op);
    void emitBranchCondition(const std::string& conditionReg, const std::string& trueLabel, const std::string& falseLabel);
    void emitLabel(const std::string& label);
    void emitUnconditionalBranch(const std::string& label);
    void emitPhiInstruction(const std::string& resultReg, const std::string& type, const std::string& trueValue, const std::string& trueLabel, const std::string& falseValue, const std::string& falseLabel);
    void emitPhiNodeForExp(const std::string& reg, const std::string& trueLabel, const std::string& falseLabel, const std::string& nextLabel);
    void emitBoolStatement(const std::string& expReg,  const std::string& rbp,  int offset);
    void emitByteStatement(const std::string& expReg,  const std::string& rbp,  int offset);


};

#endif