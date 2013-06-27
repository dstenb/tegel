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

class Param
{
	public:
		Param(const string &id, ConstantData *data)
			: id_(id), data_(data) {}

		string get_id() const { return id_; }
		const ConstantData *get() const { return data_; }

		Type type() const { return data_->get_type(); }

		void print(ostream &os) {
			os << "Param(" << id_ << ", ";
			data_->print(os);
			os << ")";
		}
	private:
		string id_;
		ConstantData *data_;
};

// TODO: rename
class ParamException : public runtime_error
{
	public:
		ParamException(const string &what) : runtime_error(what) {}
};

class Symbol
{
	public:
		Symbol(const string &name, Type t)
			: name_(name), type_(t) {}
		virtual bool is_constant() const = 0;

		string get_name() const { return name_; }
		Type get_type() const { return type_; }

		virtual void print(ostream &os) const = 0;
	private:
		string name_;
		Type type_;
};

class Argument : public Symbol
{
	public:
		Argument(const string &name, Type t)
			: Symbol(name, t)
		{
			add("cmd", new ListConstantData(StringListType));
			add("default", create_default_constant(t));
			add("info", new StringConstantData(""));
		}

		bool is_constant() const { return true; }

		Param *replace(Param *p);
		const Param *get(const string &s) const;
		void print(ostream &os) const;

	private:
		void add(const string &id, ConstantData *data) {
			assert(data != NULL);
			params_[id] = new Param(id, data);
		}

		map<string, Param *> params_;
};

class Variable : public Symbol
{
	public:
		bool is_constant() const { return false; }
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
			: parent_(parent) {}

		void add(Symbol *s);
		Symbol *lookup(const string &);
		void print(ostream &os) const;
	private:
		SymbolTable *parent_;
		map<string, Symbol *> map_;
};

#endif
