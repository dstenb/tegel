#ifndef __TYPE_H__
#define __TYPE_H__

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

class TypeFactory;

class SingleType;
class PrimitiveType;
class RecordType;
class ListType;

/** Abstract type class
 *
 * A Type object represents a type in the language (e.g. string).
 *
 * The objects contain protected constructors and destructors, and they are
 * only created by the TypeFactory. This means that each type in the language
 * corresponds to an unique object, meaning that simple pointer comparison can
 * be made to compare types in the language.
 *
 * The class contains safe methods for safe upcasting, so that RTTI doesn't
 * have to be used.
 */
class Type
{
	public:
		virtual const string &str() const = 0;

		// TODO: rename to something more suitable (dot?)
		virtual const Type *field(const string &) const {
			return nullptr;
		}

		virtual void print(ostream &os) const = 0;

		virtual const SingleType *single() const {
			return nullptr;
		}

		virtual const PrimitiveType *primitive() const {
			return nullptr;
		}

		virtual const RecordType *record() const {
			return nullptr;
		}

		virtual const ListType *list() const {
			return nullptr;
		}
	protected:
		virtual ~Type() {}
};

/**
 * A SingleType is either a primitive or a record.
 */
class SingleType : public Type
{
	public:
		virtual const SingleType *single() const { return this; }
};

class PrimitiveType : public SingleType
{
	public:
		void print(ostream &os) const;

		const string &str() const { return str_; }

		const PrimitiveType *primitive() const { return this; }
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

struct RecordField
{
	string name;
	const PrimitiveType *type;
};

class RecordType : public SingleType
{
	friend class TypeFactory;

	public:
		typedef vector<RecordField> field_vector;
		typedef field_vector::const_iterator iterator;

		const PrimitiveType *field(const string &) const;
		const PrimitiveType *field(int) const;
		const string &str() const { return str_; }
		void print(ostream &os) const;

		iterator begin() const;
		iterator end() const;

		size_t no_of_fields() const { return fields_.size(); }

		virtual const RecordType *record() const { return this; }
	protected:
		RecordType(const string &name, const field_vector &m)
			: str_(name), fields_(m) {}
	private:
		string str_;
		field_vector fields_;
};

class ListType : public Type
{
	friend class TypeFactory;

	public:
		const SingleType *elem() const { return elem_; }
		const string &str() const { return str_; }
		void print(ostream &os) const;

		virtual const ListType *list() const { return this; }
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

/**
 * The TypeFactory class is a singleton that handles the declared types in the
 * language. The class is responsible for the allocation and indexing of the
 * types.
 */
class TypeFactory
{
	public:
		static void add_record(const string &n,
			    const RecordType::field_vector &m)
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

			add_list(t);
		}

		static const Type *get(const string &s) {
			if (!initialized_)
				init();
			auto it = map_.find(s);
			return (it != map_.end()) ? it->second : nullptr;
		}

		static const ListType *get_list(const SingleType *t) {
			if (!initialized_)
				init();
			auto it = map_.find(t->str() + "[]");
			return (it != map_.end()) ?
				it->second->list() : nullptr;
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

			add_primitive(new BoolType);
			add_primitive(new IntType);
			add_primitive(new StringType);
		}

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

		static void add_primitive(PrimitiveType *p)
		{
			map_[p->str()] = p;
			add_list(p);
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
