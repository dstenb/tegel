#ifndef __PARSE_DATA_H__
#define __PARSE_DATA_H__

#include "ast.hpp"
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

class ParseContext
{
    public:
        ParseContext(const string &name, FILE *fp, bool tgp = false)
            : name(name), scanner(nullptr), data(new ParseData),
              tgp_(tgp), parsed_files_() {
            scan_init();
            scan_set(fp);
        }

        string name;
        void *scanner;
        ParseData *data;

        ParseData *get_parsed_file(const string &s) {
            auto it = parsed_files_.find(s);

            return (it != parsed_files_.end()) ? it->second : nullptr;
        }

        void set_parsed_file(const string &s, ParseData *d) {
            parsed_files_[s] = d;
        }

        map<string, ParseData *> parsed_files() {
            return parsed_files_;
        }

        bool is_tgp() const {
            return tgp_;
        }
    private:
        ParseContext(const ParseContext &) = delete;
        ParseContext &operator=(const ParseContext &) = delete;

        void scan_init();
        void scan_destroy();
        void scan_set(FILE *);

        bool tgp_;

        map<string, ParseData *> parsed_files_;
};

#endif
