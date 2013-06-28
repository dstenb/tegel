#include "type.hpp"

// TypeFactory data
bool TypeFactory::initialized_ = false;
map<string, Type*> TypeFactory::map_;

void PrimitiveType::print(ostream &os) const
{
	os << "PrimitiveType(" << str() << ")";
}

const PrimitiveType *RecordType::field(const string &f) const
{
        auto it = find_if(fields_.begin(), fields_.end(),
            [&] (const RecordField &r) { return r.name == f; });

	if (it != fields_.end())
		return (*it).type;
	return nullptr;
}

const PrimitiveType *RecordType::field(int i) const
{
	try {
		RecordField f = fields_[i];
		return f.type;
	} catch (const out_of_range &) {
		return nullptr;
	}
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
