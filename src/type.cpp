#include "type.hpp"

namespace type {

// TypeFactory data
    bool TypeFactory::initialized_ = false;
    map<string, Type*> TypeFactory::map_;

    const Type *Type::dot(const string &) const {
        return nullptr;
    }

    const SingleType *Type::single() const
    {
        return nullptr;
    }

    const PrimitiveType *Type::primitive() const
    {
        return nullptr;
    }

    const RecordType *Type::record() const
    {
        return nullptr;
    }

    const ListType *Type::list() const
    {
        return nullptr;
    }

    void Type::add_method(const TypeMethod &tm)
    {
        methods_[tm.name()] = tm;
    }

    TypeMethod Type::lookup(const string &s, const vector<const Type *> &p) const
    {
        auto it = methods_.find(s);

        if (it == methods_.end())
            throw NoSuchMethodError(str(), s);
        if (p.size() != it->second.no_of_parameters())
            throw WrongNumberOfArgumentsError(p.size(),
                                              it->second.no_of_parameters());
        if (p != it->second.parameters())
            throw WrongArgumentSignatureError(types_to_str(p),
                                              types_to_str(it->second.parameters()));
        return it->second;
    }

    TypeMethod Type::lookup(const string &s) const
    {
        return lookup(s, vector<const Type *>());
    }

    void Type::print_methods(ostream &os) const
    {
        for (auto it = methods_.begin(); it != methods_.end();
                ++it) {
            const TypeMethod &m = it->second;

            os << "\t" << m.return_type()->str() << " "
               << m.name() << "(" <<
               types_to_str(m.parameters()) << ")\n";
        }
    }

    void PrimitiveType::print(ostream &os) const
    {
        os << "PrimitiveType(" << str() << ")";
    }

    const PrimitiveType *RecordType::dot(const string &f) const
    {
        auto it = find_if(fields_.begin(), fields_.end(),
        [&] (const RecordField &r) {
            return r.name == f;
        });

        if (it != fields_.end())
            return (*it).type;
        return nullptr;
    }

    void RecordType::print(ostream &os) const
    {
        os << "RecordType(" << str() << ")\n";
        os << "  Fields:";
        for (auto it = fields_.begin(); it != fields_.end(); ++it) {
            os << "\n    " << (*it).name << "=";
            (*it).type->print(os);
        }
    }

    RecordType::iterator RecordType::begin() const
    {
        return fields_.begin();
    }

    RecordType::iterator RecordType::end() const
    {
        return fields_.end();
    }

    void RecordType::check_signature(const vector<const Type *> &v) const {

        if (v.size() != fields_.size()) {
            stringstream s;
            s << "wrong number of fields (got " << v.size()
              << ", expected " << fields_.size() << ")";
            throw UnmatchingFieldSignature(s.str());
        }

        auto m = mismatch(fields_.begin(), fields_.end(), v.begin(),
                          [] (const RecordField &f,
        const Type *t) {
            return f.type == t;
        });

        if (m.first != fields_.end()) {
            stringstream s;
            s << "wrong type for field " << (m.first - fields_.begin() + 1)
              << " (got " << (*m.second)->str() <<
              ", expected " << (*m.first).type->str() << ")";
            throw UnmatchingFieldSignature(s.str());
        }
    }

    bool RecordType::matches(const RecordType *other) const
    {
        if (no_of_fields() != other->no_of_fields())
            return false;

        auto m = mismatch(begin(), end(), other->begin(),
        [] (const RecordField &a, const RecordField &b) {
            return (a.name == b.name) && (a.type == b.type);
        });

        if (m.first != end())
            return false;
        return true;
    }

    void ListType::print(ostream &os) const
    {
        os << "ListType(" << str() << ")";
    }

    string types_to_str(const vector<const Type *> &v)
    {
        auto it = v.begin();
        stringstream sstr;

        while (it != v.end()) {
            sstr << (*it)->str();
            if (++it != v.end())
                sstr << ", ";
        }

        return sstr.str();
    }

}
