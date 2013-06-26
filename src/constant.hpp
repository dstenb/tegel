#ifndef __CONSTANT_H__
#define __CONSTANT_H__

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
	private:
		Type type_;
};

class ScalarConstantData : public ConstantData
{
	public:
		ScalarConstantData(Type t) : ConstantData(t) {}
};

class ListConstantData : public ConstantData
{
	public:
		ListConstantData(Type t) : ConstantData(t) {}

 		void add(ScalarConstantData *d);

		void print(ostream &os) const;
	private:
		vector<ScalarConstantData *> data_;
};

class BoolConstantData : public ScalarConstantData
{
	public:
		BoolConstantData(bool b)
			: ScalarConstantData(BoolType), value_(b) {}

		void print(ostream &os) const {
			os << value_;
		}
	private:
		bool value_;
};

class IntConstantData : public ScalarConstantData
{
	public:
		IntConstantData(int i)
			: ScalarConstantData(IntType), value_(i) {}

		void print(ostream &os) const {
			os << value_;
		}
	private:
		int value_;
};

class StringConstantData : public ScalarConstantData
{
	public:
		StringConstantData(const string &s)
			: ScalarConstantData(StringType), value_(s) {}

		void print(ostream &os) const {
			os << "\"" << value_ << "\"";
		}
	private:
		string value_;
};

#endif
