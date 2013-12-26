#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <cassert>
#include <map>
#include <string>
#include <stdexcept>
#include <vector>

#include "constant.hpp"
#include "type.hpp"

using namespace std;
using namespace constant;
using namespace type;

namespace symbol {

    bool is_reserved_symbol_name(const string &n);

    class Param
    {
        public:
            Param(const string &id, ConstantData *data)
                : id_(id), data_(data) {}

            ~Param() {
                delete data_;
            }

            string get_id() const {
                return id_;
            }
            const ConstantData *get() const {
                return data_;
            }
            const Type *type() const {
                return data_->type();
            }
            void print(ostream &os);
        private:
            Param(const Param &) = delete;
            Param &operator=(const Param &) = delete;

            string id_;
            ConstantData *data_;
    };

    class ParamException : public runtime_error
    {
        public:
            ParamException(const string &what) : runtime_error(what) {}
    };

    class Argument;
    class Variable;

    class Symbol
    {
        public:
            Symbol(const string &name, const Type *t)
                : name_(name), type_(t) {}

            virtual ~Symbol() {}

            /** Returns true if the symbol is read only
             *
             * With read only, it is meant that the template can not explicitly
             * change the value of the symbol. I.e. a loop variable in a for
             * loop is considered read only, even though it will be changed by
             * the underlying script.
             *
             */
            virtual bool read_only() const = 0;

            virtual void print(ostream &os) const = 0;

            virtual Argument *argument() {
                return nullptr;
            }
            virtual Variable *variable() {
                return nullptr;
            }

            string get_name() const {
                return name_;
            }
            const Type *get_type() const {
                return type_;
            }
        private:
            Symbol(const Symbol &) = delete;
            Symbol &operator=(const Symbol &) = delete;

            string name_;
            const Type *type_;
    };

    class SymbolNameError : public runtime_error
    {
        public:
            SymbolNameError(const string &n)
                : runtime_error(n + " is a reserved symbol name") {}
    };

    class Argument : public Symbol
    {
        public:
            static Argument *create(const string &n, const Type *t) {
                if (is_reserved_symbol_name(n))
                    throw SymbolNameError(n);
                return new Argument(n, t);
            }

            virtual bool read_only() const {
                return true;
            }
            virtual void print(ostream &os) const;
            virtual Argument *argument() {
                return this;
            }

            Param *replace(Param *p);
            const Param *get(const string &s) const;
        protected:
            Argument(const string &name, const Type *t)
                : Symbol(name, t), params_() {
                setup_parameters();
            }
        private:
            Argument(const Argument &) = delete;
            Argument &operator=(const Argument &) = delete;

            void add(const string &id, ConstantData *data);
            void setup_parameters();

            map<string, Param *> params_;
    };

    class Variable : public Symbol
    {
        public:
            static Variable *create(const string &n, const Type *t,
                                    bool ro = false, bool internal = false) {
                if (!internal && is_reserved_symbol_name(n))
                    throw SymbolNameError(n);
                return new Variable(n, t, ro);
            }

            virtual bool read_only() const {
                return read_only_;
            }
            virtual void print(ostream &os) const;
            virtual Variable *variable() {
                return this;
            }
        private:
            Variable(const string &name, const Type *t, bool read_only)
                : Symbol(name, t), read_only_(read_only) {}

            Variable(const Variable &) = delete;
            Variable &operator=(const Variable &) = delete;

            bool read_only_;
    };

    class SymTabAlreadyDefinedError : public runtime_error
    {
        public:
            SymTabAlreadyDefinedError(const string &what)
                : runtime_error(what) {}
    };

    class SymTabNoSuchSymbolError : public runtime_error
    {
        public:
            SymTabNoSuchSymbolError(const string &what)
                : runtime_error(what) {}
    };

    class SymbolTable
    {
        public:
            SymbolTable(SymbolTable *parent = nullptr)
                : parent_(parent), map_() {}

            /* TODO */
            ~SymbolTable() {

            }

            void add(Symbol *s);
            Symbol *lookup(const string &);
            void print(ostream &os) const;
            SymbolTable *parent() {
                return parent_;
            }
        private:
            SymbolTable(const SymbolTable &) = delete;
            SymbolTable &operator=(const SymbolTable &) = delete;

            SymbolTable *parent_;
            map<string, Symbol *> map_;
    };

}

#endif
