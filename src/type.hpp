#ifndef __TYPE_H__
#define __TYPE_H__

#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class TypeFactory;

class Type
{
	public:
		virtual const string &str() const = 0;
		bool operator==(const Type *other);

		virtual const Type *field(const string &) const {
			return nullptr;
		}

		virtual void print(ostream &os) const = 0;

		// TODO: possibly add PrimitiveType *primitive() ...
		// to avoid using dynamic_cast
	protected:
		virtual ~Type() {}
};

class SingleType : public Type
{
};

class PrimitiveType : public SingleType
{
	public:
		void print(ostream &os) const;

		const string &str() const { return str_; }
	protected:
		PrimitiveType(const string &s) : str_(s) {}
	private:
		string str_;
};

class BoolType : public PrimitiveType
{
	friend class TypeFactory;

	protected:
		BoolType() : PrimitiveType("bool") {}
};

class IntType : public PrimitiveType
{
	friend class TypeFactory;

	protected:
		IntType() : PrimitiveType("int") {}
};

class StringType : public PrimitiveType
{
	friend class TypeFactory;

	protected:
		StringType() : PrimitiveType("string") {}
};

class NoSuchFieldError : public runtime_error
{
	public:
		NoSuchFieldError(const string &f, const string &r)
			: runtime_error("No field named " + f + " in " + r) {}
};

class RecordType : public SingleType
{
	friend class TypeFactory;

	public:
		bool eql_fields(const RecordType *, // TODO
			const vector<const PrimitiveType *> &v);
		const Type *field(const string &f) const;
		const string &str() const { return str_; }
		void print(ostream &os) const;
	protected:
		RecordType(const string &name,
			const unordered_map<string, const PrimitiveType *> &m)
			: str_(name), fields_(m) {}
	private:
		string str_;
		unordered_map<string, const PrimitiveType *> fields_;
};

class ListType : public Type
{
	friend class TypeFactory;

	public:
		const SingleType *elem() const { return elem_; }
		const string &str() const { return str_; }
		void print(ostream &os) const;
	protected:
		ListType(const SingleType *t)
			: str_(t->str() + "[]"), elem_(t) {}
	private:
		string str_;
		const SingleType *elem_;
};

class TypeAlreadyDefined : public runtime_error
{
	public:
		TypeAlreadyDefined(const Type *t)
			: runtime_error("Type " + t->str() +
					" is already defined") {}
};

class TypeFactory
{
	public:
		static void add_list(const SingleType *s) {
			ListType *t = new ListType(s);

			auto it = map_.find(t->str());

			if (it == map_.end()) {
				map_[t->str()] = t;
			} else {
				delete t;
				throw TypeAlreadyDefined(it->second);
			}
		}

		static void add_record(const string &n,
			const unordered_map<string, const PrimitiveType *> &m)
		{
			// TODO: check n [a-zA-Z]...
			//
			RecordType *t = new RecordType(n, m);

			auto it = map_.find(t->str());

			if (it == map_.end()) {
				map_[t->str()] = t;
			} else {
				delete t;
				throw TypeAlreadyDefined(it->second);
			}
		}

		static const Type *get(const string &s) {
			if (!initialized_)
				init();
			auto it = map_.find(s);
			return (it != map_.end()) ? it->second : nullptr;
		}

		static void print(ostream &os) {
			for (auto it = map_.begin(); it != map_.end(); ++it) {
				it->second->print(os);
				os << "\n";
			}
		}
	private:
		static void init() {
			initialized_ = true;

			SingleType *b = new BoolType;
			SingleType *i = new IntType;
			SingleType *s = new StringType;

			map_["bool"] = b;
			add_list(b);

			map_["int"] = i;
			add_list(i);

			map_["string"] = s;
			add_list(s);
		}

		static map<string, Type *> map_;
		static bool initialized_;
};

class DifferentTypesError : public runtime_error
{
	public:
		DifferentTypesError(const Type *g, const Type *e)
			: runtime_error("Got " + g->str() +
					", expected " + e->str()) {}
};

class InvalidTypeError : public runtime_error
{
	public:
		InvalidTypeError(const string &what)
			: runtime_error(what) {}
};



#endif
