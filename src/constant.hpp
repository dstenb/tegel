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
		ConstantData(const Type *t) : type_(t) {}
		const Type *type() const { return type_; }

		virtual void print(ostream &os) const {}

		friend ostream &operator<<(ostream &os, const ConstantData &d) {
			d.print(os);
			return os;
		}
	private:
		const Type *type_;
};

class SingleConstantData : public ConstantData
{
	public:
		SingleConstantData(const Type *t) : ConstantData(t) {}
};

class PrimitiveConstantData : public SingleConstantData
{
	protected:
		PrimitiveConstantData(const Type *t)
			: SingleConstantData(t) {}
};

class ListConstantData : public ConstantData
{
	public:
		ListConstantData(const ListType *t) : ConstantData(t) {}

 		void add(SingleConstantData *d);

		void print(ostream &os) const;
	private:
		vector<SingleConstantData *> data_;
};

class BoolConstantData : public PrimitiveConstantData
{
	public:
		BoolConstantData(bool b)
			: PrimitiveConstantData(
				TypeFactory::get("bool")), value_(b) {}

		void print(ostream &os) const {
			os << value_;
		}
	private:
		bool value_;
};

class IntConstantData : public PrimitiveConstantData
{
	public:
		IntConstantData(int i)
			: PrimitiveConstantData(
				TypeFactory::get("int")), value_(i) {}

		void print(ostream &os) const {
			os << value_;
		}
	private:
		int value_;
};

class StringConstantData : public PrimitiveConstantData
{
	public:
		StringConstantData(const string &s)
			: PrimitiveConstantData(
				TypeFactory::get("string")), value_(s) {}

		void print(ostream &os) const {
			os << "\"" << value_ << "\"";
		}
	private:
		string value_;
};

// TODO add RecordConstantData

ConstantData *create_default_constant(const Type *t);

#endif
