#ifndef EX5_CODE_GEN
#define EX5_CODE_GEN

#include <string>
#include "nodes.hpp"
#include "cg.hpp"


class codeGenerator{
    int maxNumReg;

public:
    codeGenerator():maxNumReg(0){}
    std::string allocate_register();
    std::string allocateGlobalReg();
    void arithmeticCode(Exp* exp, const Exp& op1, const Exp& op2,const string& operation);


}

#endif