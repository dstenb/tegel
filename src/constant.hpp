#ifndef __CONSTANT_H__
#define __CONSTANT_H__

#include <cassert>
#include <iostream>
#include <vector>

using namespace std;

#include "type.hpp"

class ConstantData
{
	public:
		ConstantData(Type t) : type_(t) {}
		Type get_type() const { return type_; }

		virtual void print(ostream &os) const {}

		friend ostream &operator<<(ostream &os, const ConstantData &d) {
			d.print(os);
			return os;
		}
	protected:
		void set_type(Type t) { type_ = t; }
	private:
		Type type_;
};

class SingleConstantData : public ConstantData
{
	public:
		SingleConstantData(Type t) : ConstantData(t) {}
};

class ListConstantData : public ConstantData
{
	public:
		ListConstantData(Type t = EmptyList) : ConstantData(t) {
			assert(t == EmptyList || type_is_list(t)); // TODO ugly
		 }

 		void add(SingleConstantData *d);

		void print(ostream &os) const;
	private:
		vector<SingleConstantData *> data_;
};

class BoolConstantData : public SingleConstantData
{
	public:
		BoolConstantData(bool b)
			: SingleConstantData(BoolType), value_(b) {}

		void print(ostream &os) const {
			os << value_;
		}
	private:
		bool value_;
};

class IntConstantData : public SingleConstantData
{
	public:
		IntConstantData(int i)
			: SingleConstantData(IntType), value_(i) {}

		void print(ostream &os) const {
			os << value_;
		}
	private:
		int value_;
};

class StringConstantData : public SingleConstantData
{
	public:
		StringConstantData(const string &s)
			: SingleConstantData(StringType), value_(s) {}

		void print(ostream &os) const {
			os << "\"" << value_ << "\"";
		}
	private:
		string value_;
};

ConstantData *create_default_constant(Type t);

#endif
