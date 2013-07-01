#ifndef __CONSTANT_H__
#define __CONSTANT_H__

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "type.hpp"

using namespace std;
using namespace type;

namespace constant {

class ConstantData
{
	public:
		virtual const Type *type() const = 0;

		virtual void print(ostream &os) const = 0;

		friend ostream &operator<<(ostream &os, const ConstantData &d) {
			d.print(os);
			return os;
		}
};

ConstantData *create_default_constant(const Type *t);

class SingleConstantData : public ConstantData
{
	public:
		virtual const SingleType *type() const = 0;
	protected:
		SingleConstantData() = default;
};

class PrimitiveConstantData : public SingleConstantData
{
	public:
		virtual const PrimitiveType *type() const { return type_; }
	protected:
		PrimitiveConstantData(const PrimitiveType *t)
			: SingleConstantData(), type_(t) {}

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

class UnevenNoOfFieldsException : public runtime_error
{
	public:
		UnevenNoOfFieldsException(const string &what)
			: runtime_error(what) {}
};

// TODO: add fields_, set(), etc.
class RecordConstantData : public SingleConstantData
{
	public:
		RecordConstantData(const RecordType *t)
			: SingleConstantData(), type_(t) {
			for (auto it = t->begin(); it != t->end(); ++it) {
				RecordField f = (*it);
				values_.push_back(
					static_cast<PrimitiveConstantData *>(
						create_default_constant(f.type)));
			}
		}

		RecordConstantData(const RecordType *t,
				vector<PrimitiveConstantData *> &v)
			: SingleConstantData(), type_(t), values_(v) {}

		virtual const RecordType *type() const { return type_; }

		void print(ostream &os) const;
	private:
		const RecordType *type_;
		vector<PrimitiveConstantData *> values_;
};

void validate_field_types(const RecordType *t,
		const vector<PrimitiveConstantData *> &v);

}

#endif
