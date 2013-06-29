#include "constant.hpp"

void ListConstantData::add(SingleConstantData *d)
{
	if (d->type() != type()->elem())
		throw DifferentTypesError(d->type(), type()->elem());
	data_.push_back(d);
}

void ListConstantData::print(ostream &os) const
{
	os << "[";

	auto it = data_.begin();

	while (it != data_.end()) {
		(*it)->print(os);
		if (++it != data_.end())
			os << ", ";
	}

	os << "]";
}

void RecordConstantData::print(ostream &os) const
{
	os << "{";

	auto it = values_.begin();

	while (it != values_.end()) {
		(*it)->print(os);
		if (++it != values_.end())
			os << ", ";
	}

	os << "}";
}

ConstantData *create_default_constant(const Type *t)
{
	if (t == TypeFactory::get("bool"))
		return new BoolConstantData(false);
	else if (t == TypeFactory::get("int"))
		return new IntConstantData(0);
	else if (t == TypeFactory::get("string"))
		return new StringConstantData("");
	else if (t->list())
		return new ListConstantData(t->list());
	else if (t->record())
		return new RecordConstantData(t->record());
	else
		return nullptr;
}

// TODO: fix
void validate_field_types(const RecordType *t,
		const vector<PrimitiveConstantData *> &v)
{
	if (v.size() != t->no_of_fields()) {
		stringstream s;
		s << "Uneven number of fields (got " << v.size()
			<< ", expected " << t->no_of_fields() << ")";
		throw UnevenNoOfFieldsException(s.str());
	}

	// TODO rewrite
	int i = 0;
	for (auto it = t->begin(); it != t->end(); ++it, ++i) {
		RecordField r = (*it);
		if (r.type != v[i]->type()) // TODO: improve error
			throw DifferentTypesError(v[i]->type(), r.type);
	}
}
