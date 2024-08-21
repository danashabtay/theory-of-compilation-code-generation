#ifndef EX5_CODE_GEN
#define EX5_CODE_GEN

#include <string>
#include "nodes.hpp"
#include "cg.hpp"

using namespace std;


class codeGenerator{
    int maxNumReg;

public:
    codeGenerator():maxNumReg(0){}
    string allocateReg(bool global)();
    string relopGetter(const string &operation);
    string binopGetter(const string &operation);
    void globalCode();
    void binopCode(Exp *exp, const Exp &op1, const Exp &op2, const string &op);
    void relopCode(Exp *exp, const Exp *op1, const Exp *op2, const string &op);

}

#endif