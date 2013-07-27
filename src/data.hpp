#ifndef __PARSE_DATA_H__
#define __PARSE_DATA_H__

#include "symbol.hpp"
#include "type.hpp"

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

class ParseContextLoadError : public runtime_error
{
    public:
        ParseContextLoadError(const string &what):
            runtime_error(what) {}
};

class ParseContext
{
    public:
        ParseContext(const string &path, bool tgp = false)
            : path(path), scanner(nullptr), data(new ParseData), tgp_(tgp) {
            scan_init();
        }

        string path;
        void *scanner;
        ParseData *data;

        void load() {
            if (path.empty())
                scan_stdin();
            else
                scan_load(path.c_str());
        }

        bool is_tgp() const {
            return tgp_;
        }
    private:
        ParseContext(const ParseContext &) = delete;
        ParseContext &operator=(const ParseContext &) = delete;

        void scan_init();
        void scan_destroy();
        void scan_load(const char *);
        void scan_stdin();

        bool tgp_;
};

#endif
