
#ifndef _236360_3_SYMTABLE_H
#define _236360_3_SYMTABLE_H

#include <vector>
#include <string>



class symTableEntry
{
public:
    std::string name;
    std::string type;
    int offset;
    bool isFunc;
    std::string params;

    symTableEntry(std::string name, std::string type, int offset, bool isFunc, std::string params);
    ~symTableEntry() = default;
};

class symTable
{
public:
    std::vector<symTableEntry *> symbolsTable;
    int curr_offset;
    bool loop;

    symTable(int curr_offset, bool loop);
    ~symTable();
    bool doesSymbolExists(const std::string name);
    void addSymbol(const symTableEntry &symbol);
};

class symTableStack
{
    public:

    std::vector<symTable *> tableStack;
    std::vector<int> offsetStack;

    symTableStack();
    ~symTableStack();
    void addTable(bool loop);
    void removeTable();
    void insertSymbol(std::string name, std::string type, bool isFunction, std::string params = "");
    bool doesSymbolExists(const std::string name);
    symTable *getCurrSymTable();
    void printScope();
    symTableEntry* getSymbol(const std::string name);
    bool is_loop();
};

#endif //_236360_3_SYMTABLE_H