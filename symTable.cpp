#include "symTable.hpp"
#include "hw3_output.hpp"
#include <iostream>
#include <string>
#include <algorithm>

std::string toUpperCase(const std::string &input) 
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

/* symTableEntry */

symTableEntry::symTableEntry(std::string name, std::string type, int offset, bool isFunc, std::string params) : name(name), type(type), offset(offset), isFunc(isFunc), params(params) {}

/* symTable */

symTable::symTable(int curr_offset, bool loop) : curr_offset(curr_offset), loop(loop) {}

symTable::~symTable()
{
    for (auto it = symbolsTable.begin(); it != symbolsTable.end(); it++)
    {
        delete (*it);
    }
}

bool symTable::doesSymbolExists(const std::string name)
{
    std::cout << "Checking scope of size " << symbolsTable.size() << "\n";
    for(int i = 0; i < symbolsTable.size(); i++){
        std::cout << "iteration " << i << " Symbol is:" << symbolsTable[i]->name << "\n";
        if (symbolsTable[i]->name == name)
            return true;
    }
    return false;
}

void symTable::addSymbol(const symTableEntry &symbol)
{
    symbolsTable.push_back(new symTableEntry(symbol));
    if (symbol.offset > 0)
    {
        curr_offset = symbol.offset;
    }
}

/* symTableStack */

symTableStack::symTableStack() : tableStack(), offsetStack()
{
    offsetStack.push_back(0);
    // creating global scope:
    
    addTable(false);
    insertSymbol("print", "void", true, "string");
    insertSymbol("printi", "void", true, "int");
    insertSymbol("readi", "int", true, "int");


}

symTableStack::~symTableStack()
{
    for (auto it = tableStack.begin(); it != tableStack.end(); it++)
    {
        symTable *curr_table = *it;
        delete curr_table;
    }
}

void symTableStack::addTable(bool loop)
{
    symTable *newTable = new symTable(offsetStack.back(), loop);
    tableStack.push_back(newTable);
    offsetStack.push_back(offsetStack.back());
}

void symTableStack::removeTable()
{
    symTable *curr_table = tableStack.back();
    output::endScope();

    /* printing symbols:
    for (auto it = (*curr_table).symbolsTable.begin(); it != (*curr_table).symbolsTable.end(); it++)
    {
        if ((*it)->isFunc)
        {
            output::printID((*it)->name, (*it)->offset, toUpperCase(output::makeFunctionType((*it)->params, (*it)->type)));
        }
        else
        {
            output::printID((*it)->name, (*it)->offset, toUpperCase((*it)->type));
        }
    }
    */
    tableStack.pop_back();
    offsetStack.pop_back();
    delete curr_table;
}

void symTableStack::insertSymbol(std::string name, std::string type, bool isFunction, std::string params)
{
    symTable *curr_table = tableStack.back();
    int offset;
    if (isFunction)
    {
        offset = 0;
    }
    else
    {
        offset = offsetStack.back();
        offsetStack.back() = offsetStack.back() + 1;
    }
    symTableEntry symbol = symTableEntry(name, type, offset, isFunction, params);
    curr_table->addSymbol(symbol);
}

bool symTableStack::doesSymbolExists(const std::string name)
{
    return this->getSymbol(name) != nullptr;
}

symTableEntry* symTableStack::getSymbol(const std::string name)
{
    for (const auto & curr_table : tableStack)
    {
        if (!curr_table)
        {
            continue;
        }
        for (const auto & symbol : curr_table->symbolsTable)
        {
            if (symbol->name == name)
            {
                return symbol;
            }    
        }
    }
    return nullptr;
}

bool symTableStack::is_loop()
{   
    for (auto it = tableStack.rbegin(); it != tableStack.rend(); ++it) {
        symTable *current = *it;
        if (current->loop)
            return true;
    }
    return false;
}

symTable *symTableStack::getCurrSymTable()
{
    return tableStack.back();
}

void symTableStack::printScope()
{
}

void symTableStack::cleanUp(){
    while(!tableStack.empty())
    {
        tableStack.removeTable();
    }
}
