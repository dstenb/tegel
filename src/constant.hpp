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
		virtual const Type *type() const = 0;

		virtual void print(ostream &os) const {}

		friend ostream &operator<<(ostream &os, const ConstantData &d) {
			d.print(os);
			return os;
		}
};

class SingleConstantData : public ConstantData
{
	public:
		virtual const SingleType *type() const = 0;
	protected:
		SingleConstantData() = default;
};

class PrimitiveConstantData : public SingleConstantData
{
	protected:
		PrimitiveConstantData(const PrimitiveType *t)
			: SingleConstantData(), type_(t) {}

		virtual const PrimitiveType *type() const { return type_; }
	private:
		const PrimitiveType *type_;
};

class ListConstantData : public ConstantData
{
	public:
		ListConstantData(const ListType *t) : type_(t) {}

		virtual const ListType *type() const { return type_; }

 		void add(SingleConstantData *d);

		void print(ostream &os) const;
	private:
		const ListType *type_;
		vector<SingleConstantData *> data_;
};

class BoolConstantData : public PrimitiveConstantData
{
	public:
		BoolConstantData(bool b)
			: PrimitiveConstantData(
				static_cast<const PrimitiveType *>(
					TypeFactory::get("bool"))), value_(b) {}

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
				static_cast<const PrimitiveType *>(
					TypeFactory::get("int"))), value_(i) {}

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
				static_cast<const PrimitiveType *>(
					TypeFactory::get("string"))), value_(s) {}


		void print(ostream &os) const {
			os << "\"" << value_ << "\"";
		}
	private:
		string value_;
};

// TODO: add fields_, set(), etc.
class RecordConstantData : public SingleConstantData
{
	public:
		RecordConstantData(const RecordType *t)
			: SingleConstantData(), type_(t) {}

		virtual const RecordType *type() const { return type_; }

		void print(ostream &os) const {
			// TODO
		}
	private:
		const RecordType *type_;
};

ConstantData *create_default_constant(const Type *t);

#endif
