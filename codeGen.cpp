#include "codeGen.hpp"
#include <iostream>
using namespace std;


std::string codeGen::allocateReg(){
    std::string newReg = "%var_" + std::to_string(maxNumReg++);
    return newReg;
}

std::string codeGen::allocateGlobalReg(){
    std::string newGreg = "@var_" + std::to_string(maxNumReg++);
    return newGreg;
}