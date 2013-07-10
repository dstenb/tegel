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

class ConstantData;
class ConstantDataVisitor;
class BoolConstantData;
class IntConstantData;
class StringConstantData;
class ListConstantData;
class RecordConstantData;

class ConstantData
{
	public:
		virtual ~ConstantData() {}

		/** Returns the constant's type
		 *
		 */
		virtual const Type *type() const = 0;

		/** Print the constant to the stream
		 *
		 */
		virtual void print(ostream &) const = 0;

		/** Accept the ConstantDataVisitor (for the visitor pattern)
		 *
		 */
		virtual void accept(ConstantDataVisitor &) const = 0;

		friend ostream &operator<<(ostream &os, const ConstantData &d) {
			d.print(os);
			return os;
		}
};

/** ConstantData visitor interface
 *
 */
class ConstantDataVisitor
{
	public:
		virtual void visit(const BoolConstantData *) = 0;
		virtual void visit(const IntConstantData *) = 0;
		virtual void visit(const StringConstantData *) = 0;
		virtual void visit(const ListConstantData *) = 0;
		virtual void visit(const RecordConstantData *) = 0;
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
		PrimitiveConstantData(const PrimitiveConstantData &) = delete;
		PrimitiveConstantData &operator=(const
				PrimitiveConstantData &) = delete;

		const PrimitiveType *type_;
};

class ListConstantData : public ConstantData
{
	public:
		ListConstantData(const ListType *t) : type_(t), data_() {}

		~ListConstantData() {
			for (auto it = begin(); it != end(); ++it)
				delete (*it);
		}

		typedef vector<SingleConstantData *>::const_iterator
			iterator;

		virtual const ListType *type() const { return type_; }
		virtual void print(ostream &) const;
		virtual void accept(ConstantDataVisitor &v) const { v.visit(this);}

		/** Add a constant to the end of the list. Throws if wrong type
		 *
		 * @throw DifferentTypesError
		 */
 		void add(SingleConstantData *d);

		iterator begin() const { return data_.begin(); }
		iterator end() const { return data_.begin(); }
	private:
		ListConstantData(const ListConstantData &) = delete;
		ListConstantData &operator=(const ListConstantData &) = delete;

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

		virtual void print(ostream &) const;
		virtual void accept(ConstantDataVisitor &v) const { v.visit(this);}

		/** Returns the boolean value
		 *
		 */
		bool value() const { return value_; }
	private:
		BoolConstantData(const BoolConstantData &) = delete;
		BoolConstantData &operator=(const BoolConstantData &) = delete;

		bool value_;
};

class IntConstantData : public PrimitiveConstantData
{
	public:
		IntConstantData(int i)
			: PrimitiveConstantData(
				static_cast<const PrimitiveType *>(
					TypeFactory::get("int"))), value_(i) {}

		virtual void print(ostream &) const;
		virtual void accept(ConstantDataVisitor &v) const { v.visit(this);}

		/** Returns the integer value
		 *
		 */
		int value() const { return value_; }
	private:
		IntConstantData(const IntConstantData &) = delete;
		IntConstantData &operator=(const IntConstantData &) = delete;

		int value_;
};

class StringConstantData : public PrimitiveConstantData
{
	public:
		StringConstantData(const string &s)
			: PrimitiveConstantData(
				static_cast<const PrimitiveType *>(
					TypeFactory::get("string"))), value_(s) {}

		virtual void print(ostream &) const;
		virtual void accept(ConstantDataVisitor &v) const { v.visit(this);}

		/** Returns the string
		 *
		 */
		string value() const { return value_; }
	private:
		StringConstantData(const StringConstantData &) = delete;
		StringConstantData &operator=(const StringConstantData &) = delete;

		string value_;
};

class UnmatchingFieldSignature : public runtime_error
{
	public:
		UnmatchingFieldSignature(const string &what)
			: runtime_error(what) {}
};

class RecordConstantData : public SingleConstantData
{
	public:
		RecordConstantData(const RecordType *t)
			: SingleConstantData(), type_(t), values_() {
			set_default();
		}

		~RecordConstantData() {
			clear();
		}

		typedef vector<PrimitiveConstantData *>::const_iterator
			iterator;

		virtual const RecordType *type() const { return type_; }
		virtual void print(ostream &) const;
		virtual void accept(ConstantDataVisitor &v) const { v.visit(this);}

		/** Set the fields. The object will take over ownership of the
		 * constants
		 */
		void set(const vector<PrimitiveConstantData *> &);

		iterator begin() const { return values_.begin(); }
		iterator end() const { return values_.begin(); }
	private:
		RecordConstantData(const RecordConstantData &) = delete;
		RecordConstantData &operator=(const RecordConstantData &) = delete;

		void clear();
		void set_default();

		const RecordType *type_;
		vector<PrimitiveConstantData *> values_;
};

}

#endif
