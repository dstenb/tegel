#ifndef __PARSE_DATA_H__
#define __PARSE_DATA_H__

#include "symbol.hpp"
#include "type.hpp"

class ParseContext
{
    public:
        ParseContext(bool tgp = false)
            : scanner(nullptr), tgp_(tgp) {
            scan_init();
        }

        void *scanner;
    private:
        void scan_init();
        void scan_destroy();
        void scan_load(const char *);

        bool tgp_;
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
