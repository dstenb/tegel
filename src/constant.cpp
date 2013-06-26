#include "constant.hpp"

void ListConstantData::add(ScalarConstantData *d)
{
	if (d->get_type() != get_type())
		throw InvalidTypeError(d->get_type(), get_type());
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
			return new ListConstantData(t);
		default:
			return NULL;
			break;
	}
}
