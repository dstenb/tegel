#include "constant.hpp"

void ListConstantData::add(ScalarConstantData *d)
{
	// Specify the list for a specific scalar type
	if (get_type() == EmptyList)
		set_type(type_scalar_to_list(d->get_type()));

	if (d->get_type() != type_list_to_scalar(get_type()))
		throw DifferentTypesError(d->get_type(),
			    type_list_to_scalar(get_type()));
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

ConstantData *create_default_constant(Type t)
{
	switch (t) {
		case BoolType:
			return new BoolConstantData(false);
		case IntType:
			return new IntConstantData(0);
		case StringType:
			return new StringConstantData("");
		case BoolListType:
		case IntListType:
		case StringListType:
		case EmptyList:
			return new ListConstantData(t);
		default:
			return nullptr;
	}
}
