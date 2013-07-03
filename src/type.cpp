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

void PrimitiveType::print(ostream &os) const
{
	os << "PrimitiveType(" << str() << ")";
}

const PrimitiveType *RecordType::dot(const string &f) const
{
        auto it = find_if(fields_.begin(), fields_.end(),
            [&] (const RecordField &r) { return r.name == f; });

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

void ListType::print(ostream &os) const
{
	os << "ListType(" << str() << ")";
}

}
