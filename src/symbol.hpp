#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <map>
#include <string>
#include <vector>

#include "constant.hpp"
#include "type.hpp"

using namespace std;

class Param
{
	public:
		Param(const string &id, const ConstantData &data)
			: id_(id), data_(data) {}

		string get_id() const;
		ConstantData *get() const;
		void set(const ConstantData &);
	private:
		string id_;
		ConstantData data_;
};

class Symbol
{
	public:
		Symbol(const string &name, Type t)
			: name_(name), type_(t) {}
		virtual bool is_constant() const = 0;

		string get_name() const;
	private:
		string name_;
		Type type_;
};

class Argument : public Symbol
{
	public:
		bool is_constant() const { return true; }

		void add(Param *p);
		Param *get(const string &);
};

class Variable : public Symbol
{
	public:
		bool is_constant() const { return false; }
};

class SymbolTable
{
	public:
		SymbolTable(SymbolTable *parent = nullptr) : parent_(parent) {}

		void add(Symbol *s);
		Symbol *lookup(const string &);
	private:
		SymbolTable *parent_;
		map<string, Symbol *> map_;
};

#endif
