#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <stdexcept>
#include <vector>

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

#endif
