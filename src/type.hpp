#ifndef __TYPE_H__
#define __TYPE_H__

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

namespace type {

    class TypeFactory;

    class Type;
    class SingleType;
    class PrimitiveType;
    class BoolType;
    class IntType;
    class StringType;
    class RecordType;
    class ListType;

    /** Type Method class
     *
     * A TypeMethod object represents a method prototype for a type. It has a
     * defined name, return type and a vector of parameter (types)
     */
    class TypeMethod
    {
        public:
            TypeMethod(const string &name, const Type *rt,
                       vector<const Type *> &params)
                : name_(name), return_(rt), params_(params) {}
            TypeMethod()
                : name_(""), return_(nullptr), params_() {}
            TypeMethod(const TypeMethod &) = default;
            TypeMethod &operator=(const TypeMethod &) = default;

            string name() const {
                return name_;
            }
            const Type *return_type() const {
                return return_;
            }
            vector<const Type *> parameters() const {
                return params_;
            }
            size_t no_of_parameters() const {
                return params_.size();
            }
        private:
            string name_;
            const Type *return_;
            vector<const Type *> params_;
    };

    class NoSuchMethodError : public runtime_error
    {
        public:
            NoSuchMethodError(const string &t, const string &m)
                : runtime_error(t +  " has no method named " + m) {}
    };

    class WrongNumberOfArgumentsError : public runtime_error
    {
        public:
            WrongNumberOfArgumentsError(size_t g, size_t e)
                : runtime_error("wrong number of arguments (got " +
                                to_string(g) + + ", expected "+
                                to_string(e) + ")") {}
    };

    class WrongArgumentSignatureError : public runtime_error
    {
        public:
            WrongArgumentSignatureError(const string &g, const string &e)
                : runtime_error("wrong argument signature (got [" + g
                                + "], expected [" + e + "])") {}
    };

    /** Convert a vector of types to a comma delimited string
     *
     */
    string types_to_str(const vector<const Type *> &);

    class TypeVisitor
    {
        public:
            virtual ~TypeVisitor() {}
            virtual void visit(const BoolType *) = 0;
            virtual void visit(const IntType *) = 0;
            virtual void visit(const StringType *) = 0;
            virtual void visit(const ListType *) = 0;
            virtual void visit(const RecordType *) = 0;
    };

    /** Abstract type class
     *
     * A Type object represents a type in the language (e.g. string). A type acts
     * similarly to an object in a OO language, i.e. they have a set of associated
     * methods that is callable
     *
     * The objects contain protected constructors and destructors, and they are
     * only created by the friend TypeFactory. This means that each type in the
     * language corresponds to an unique object, meaning that simple pointer
     * comparison can be made to compare types in the language.
     *
     * The class contains safe methods for safe upcasting, so that RTTI doesn't
     * have to be used.
     */
    class Type
    {
            friend class TypeFactory;

        public:
            /** Get the type's string representation
             *
             */
            virtual string str() const = 0;

            /** Dot action resolution
             *
             * Returns the resulting type when the dot action is applied to
             * a type (e.g. field access in a record)
             *
             * @return The resulting type. Returns nullptr if not
             * applicable to the type
             */
            virtual const Type *dot(const string &) const;

            /** Prints the Type */
            virtual void print(ostream &os) const = 0;

            /** Safe SingleType* caster (alternative to RTTI)
             * @return Pointer if successful, nullptr if not
             */
            virtual const SingleType *single() const;

            /** Safe PrimitiveType* caster (alternative to RTTI)
             * @return Pointer if successful, nullptr if not
             */
            virtual const PrimitiveType *primitive() const;

            /** Safe RecordType* caster (alternative to RTTI)
             * @return Pointer if successful, nullptr if not
             */
            virtual const RecordType *record() const;

            /** Safe ListType* caster (alternative to RTTI)
             * @return Pointer if successful, nullptr if not
             */
            virtual const ListType *list() const;

            /** Lookup a method
             *
             * @return The TypeMethod class if found, throws if not found
             * @throw NoSuchMethodError
             */
            TypeMethod lookup(const string &,
                              const vector<const Type *> &) const;

            /** Lookup a parameter-less method
             *
             * @return The TypeMethod class if found, throws if not found
             * @throw NoSuchMethodError
             */
            TypeMethod lookup(const string &) const;

            /** Print the list of methods defined for the type
             *
             */
            void print_methods(ostream &os) const;


            virtual void accept(TypeVisitor &) const = 0;
        protected:
            Type() : methods_() {}
            virtual ~Type() {}

            void add_method(const TypeMethod &tm);
        private:
            map<string, TypeMethod> methods_;
    };

    /**
     * A SingleType is either a primitive or a record.
     */
    class SingleType : public Type
    {
        public:
            virtual const SingleType *single() const {
                return this;
            }
        protected:
            virtual ~SingleType() {}
    };

    class PrimitiveType : public SingleType
    {
        public:
            virtual void print(ostream &os) const;
            virtual string str() const {
                return str_;
            }

            virtual const PrimitiveType *primitive() const {
                return this;
            }
        protected:
            PrimitiveType(const string &s) : str_(s) {}
            virtual ~PrimitiveType() {}
        private:
            string str_;
    };

    class BoolType : public PrimitiveType
    {
            friend class TypeFactory;

        public:
            virtual void accept(TypeVisitor &v) const {
                v.visit(this);
            }
        protected:
            BoolType() : PrimitiveType("bool") {}
            virtual ~BoolType() {}
    };

    class IntType : public PrimitiveType
    {
            friend class TypeFactory;

        public:
            virtual void accept(TypeVisitor &v) const {
                v.visit(this);
            }
        protected:
            IntType() : PrimitiveType("int") {}
            virtual ~IntType() {}
    };

    class StringType : public PrimitiveType
    {
            friend class TypeFactory;

        public:
            virtual void accept(TypeVisitor &v) const {
                v.visit(this);
            }
        protected:
            StringType() : PrimitiveType("string") {}
            virtual ~StringType() {}
    };

    class NoSuchFieldError : public runtime_error
    {
        public:
            NoSuchFieldError(const string &f, const string &r)
                : runtime_error("no field named " + f + " in " + r) {}
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

            /** Dot action resolution
             *
             * Returns the resulting type for a field lookup
             *
             * @return The resulting type.
             * @throw NoSuchFieldError if no field with the given name is found
             */
            virtual const PrimitiveType *dot(const string &) const;

            virtual string str() const {
                return str_;
            }

            virtual void print(ostream &os) const;

            iterator begin() const;
            iterator end() const;

            /** Returns the number of fields in the record
             *
             */
            size_t no_of_fields() const {
                return fields_.size();
            }

            virtual const RecordType *record() const {
                return this;
            }

            virtual void accept(TypeVisitor &v) const {
                v.visit(this);
            }

            /** Returns true if the record types have the same field signature
             *
             */
            bool matches(const RecordType *) const;
        protected:
            RecordType(const string &name, const field_vector &m)
                : str_(name), fields_(m) {}
            virtual ~RecordType() {}
        private:
            string str_;
            field_vector fields_;
    };

    class ListType : public Type
    {
            friend class TypeFactory;

        public:
            /** Returns the list's element type
             *
             */
            virtual const SingleType *elem() const {
                return elem_;
            }
            virtual string str() const {
                return str_;
            }
            virtual void print(ostream &os) const;

            virtual const ListType *list() const {
                return this;
            }

            virtual void accept(TypeVisitor &v) const {
                v.visit(this);
            }
        protected:
            ListType(const SingleType *t)
                : str_(t->str() + "[]"), elem_(t) {}
            ListType(const ListType &) = default;
            ListType &operator=(const ListType &) = default;
            virtual ~ListType() {}
        private:
            string str_;
            const SingleType *elem_;
    };

    class TypeAlreadyDefined : public runtime_error
    {
        public:
            TypeAlreadyDefined(const Type *t)
                : runtime_error("type '" + t->str() +
                                "' is already defined") {}
    };

    /** The TypeFactory class is a singleton that handles the declared types in
     * the language. The class is responsible for the allocation and indexing
     * of the types.
     *
     */
    class TypeFactory
    {
        public:
            /** Add a record type to the factory. A corresponding list type
             * will also be created and added.
             *
             * If an identical record, i.e. same name and field signature
             * (same field names and types), is already defined, then nothing
             * will be done.
             *
             */
            static void add_record(const string &n,
                                   const RecordType::field_vector &m)
            {
                RecordType *t = new RecordType(n, m);

                auto it = map_.find(t->str());

                if (it == map_.end()) {
                    map_[t->str()] = t;
                } else {
                    if (it->second->record()) {
                        if (it->second->record()->matches(t)) {
                            /* An equivalent record is already defined, don't
                             * do anything */
                            return;
                        } else {
                            delete t;
                            /* TODO: throw better exception */
                            throw TypeAlreadyDefined(it->second);
                        }
                    } else {
                        delete t;
                        throw TypeAlreadyDefined(it->second);
                    }
                }

                auto l = add_list(t);
                setup_record_list_methods(l);
            }

            /** Lookup a type from a string
             *
             * @return The type if found, else nullptr
             */
            static const Type *get(const string &s) {
                if (!initialized_)
                    init();
                auto it = map_.find(s);
                return (it != map_.end()) ? it->second : nullptr;
            }

            /** Returns the corresponding list type from a single type
             *
             * @return The type if found, else nullptr
             */
            static const ListType *get_list(const SingleType *t) {
                if (!initialized_)
                    init();
                auto it = map_.find(t->str() + "[]");
                return (it != map_.end()) ?
                       it->second->list() : nullptr;
            }

            /** Prints the type map to the given stream
             *
             */
            static void print(ostream &os) {
                if (!initialized_)
                    init();
                for (auto it = map_.begin(); it != map_.end(); ++it) {
                    it->second->print(os);
                    os << "\n";
                    it->second->print_methods(os);
                }
            }
        private:
            static void init() {
                initialized_ = true;
                setup_primitives();
            }

            static ListType *add_list(const SingleType *s) {
                ListType *t = new ListType(s);

                auto it = map_.find(t->str());

                if (it == map_.end()) {
                    map_[t->str()] = t;
                    return t;
                } else {
                    delete t;
                    throw TypeAlreadyDefined(it->second);
                }
            }

            static void add_primitive(PrimitiveType *p)
            {
                map_[p->str()] = p;
            }

            static void setup_primitives() {
                auto b = new BoolType;
                auto i = new IntType;
                auto s = new StringType;

                map_[b->str()] = b;
                map_[i->str()] = i;
                map_[s->str()] = s;

                auto bl = add_list(b);
                auto il = add_list(i);
                auto sl = add_list(s);

                vector<const Type *> e_v = { };
                vector<const Type *> b_v = { b };
                vector<const Type *> i_v = { i };
                vector<const Type *> s_v = { s };
                vector<const Type *> ss_v = { s, s };

                /* bool methods */
                b->add_method(TypeMethod("str", s, e_v));

                /* int methods */
                i->add_method(TypeMethod("downto", il, i_v));
                i->add_method(TypeMethod("str", s, e_v));
                i->add_method(TypeMethod("upto", il, i_v));

                /* string methods */
                s->add_method(TypeMethod("lalign", s, i_v));
                s->add_method(TypeMethod("length", i, e_v));
                s->add_method(TypeMethod("lower", s, e_v));
                s->add_method(TypeMethod("ralign", s, i_v));
                s->add_method(TypeMethod("upper", s, e_v));
                s->add_method(TypeMethod("title", s, e_v));
                s->add_method(TypeMethod("replace", s, ss_v));
                s->add_method(TypeMethod("wrap", sl, i_v));

                /* bool[] methods */
                bl->add_method(TypeMethod("size", i, e_v));

                /* int[] methods */
                il->add_method(TypeMethod("size", i, e_v));
                il->add_method(TypeMethod("sort", il, b_v));

                /* string[] methods */
                sl->add_method(TypeMethod("join", s, s_v));
                sl->add_method(TypeMethod("size", i, e_v));
                sl->add_method(TypeMethod("sort", sl, b_v));
            }

            static void setup_record_list_methods(ListType *t) {
                vector<const Type *> sb_v = { get("string"),
                                              get("bool")
                                            };
                vector<const Type *> s_v = { get("string") };

                t->add_method(TypeMethod("join", get("string"), s_v));
                t->add_method(TypeMethod("sort", t, sb_v));
            }

            static map<string, Type *> map_;
            static bool initialized_;
    };

    class DifferentTypesError : public runtime_error
    {
        public:
            DifferentTypesError(const Type *g, const Type *e)
                : runtime_error("got " + g->str() +
                                ", expected " + e->str()) {}
    };

    class InvalidTypeError : public runtime_error
    {
        public:
            InvalidTypeError(const string &what)
                : runtime_error(what) {}
    };

}

#endif
