#include "_type.hpp"

// TypeFactory data
bool TypeFactory::initialized_ = false;
map<string, Type*> TypeFactory::map_;

void PrimitiveType::print(ostream &os) const
{
	os << "PrimitiveType(" << str() << ")";
}

const Type *RecordType::field(const string &f) const
{
	auto it = fields_.find(f);

	if (it == fields_.end())
		throw NoSuchFieldError(f, str());
	return it->second;
}

void RecordType::print(ostream &os) const
{
	os << "RecordType(" << str() << ")\n";
	os << "  Fields:";
	for (auto it = fields_.begin(); it != fields_.end();
			++it) {
		os << "\n    " << it->first << "=";
		it->second->print(os);
	}
}

void ListType::print(ostream &os) const
{
	os << "ListType(" << str() << ")";
}
