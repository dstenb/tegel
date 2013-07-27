#ifndef __PARSE_DATA_H__
#define __PARSE_DATA_H__

#include "symbol.hpp"
#include "type.hpp"

struct ParseContext
{
    void *scanner;
};

struct ParseData
{
        ParseData()
            : root_table(new symbol::SymbolTable),
              current_table(root_table),
              arguments(), body(nullptr) {}

        symbol::SymbolTable *root_table;
        symbol::SymbolTable *current_table;
        vector<symbol::Argument *> arguments;
        ast::Statements *body;

    private:
        ParseData(const ParseData &) = delete;
        ParseData &operator=(const ParseData &) = delete;
};

#endif
