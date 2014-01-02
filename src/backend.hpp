#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <stdexcept>
#include <vector>

#include <stdarg.h>

using namespace std;

#include "ast.hpp"
#include "data.hpp"
#include "symbol.hpp"

class Backend
{
    public:
        virtual ~Backend() {}

        virtual void generate(ostream &,
                              const vector<symbol::Argument *> &,
                              ast::Statements *) = 0;
};

class TgpBackend
{
    public:
        virtual ~TgpBackend() {}

        virtual void generate(ostream &,
                              ParseData *tgp_data,
                              map<string, ParseData *> &tgl_data) = 0;
};

/** Symbol table for untyped backends
 *
 * The symbol table maps symbols to strings created by calls to
 * StringCreator.next()
 *
 */
template<typename StringCreator>
class BackendUntypedSymbolTable
{
    public:
        BackendUntypedSymbolTable()
            : creator_(), map_() {}

        virtual ~BackendUntypedSymbolTable() {}

        string get(symbol::Symbol *s) {
            auto it = map_.find(s);
            string str;
            if (it != map_.end())
                str = it->second;
            else
                str = map_[s] = creator_.next();
            return str;
        }

    private:
        StringCreator creator_;
        map<symbol::Symbol *, string> map_;
};

class BackendException : public runtime_error
{
    public:
        BackendException(const string &s)
            : runtime_error(s) {}
};

class UnknownBackend : public runtime_error
{
    public:
        UnknownBackend(const string &s)
            : runtime_error(s) {}
};

class BackendGenerator : public ast::AST_Visitor
{
    public:
        BackendGenerator(ostream &os)
            : os_(os) {}

        void writev(const char *fmt, va_list val) {
            while (*fmt) {
                if (*fmt == '%') {
                    ast::AST_Node *n;
                    fmt++;
                    switch (*fmt) {
                    case '%':
                        os_ << "%";
                        break;
                    case 'a':
                        n = va_arg(val, ast::AST_Node *);
                        if (n)
                            n->accept(*this);
                        break;
                    case 's':
                        os_ << va_arg(val, char *);
                        break;
                    case 'i':
                        os_ << va_arg(val, int);
                        break;
                    default:
                        os_ << "%" << *fmt;
                        break;
                    }
                    fmt++;
                } else {
                    os_ << *fmt++;
                }
            }
        }

        void write(const char *fmt, ...) {
            va_list val;

            va_start(val, fmt);
            writev(fmt, val);
            va_end(val);
        }
    private:
        ostream &os_;
};

#endif
