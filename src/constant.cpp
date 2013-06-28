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

ConstantData *create_default_constant(const Type *t)
{
	const ListType *l;
	const RecordType *r;

	if (t == TypeFactory::get("bool"))
		return new BoolConstantData(false);
	else if (t == TypeFactory::get("int"))
		return new IntConstantData(0);
	else if (t == TypeFactory::get("string"))
		return new StringConstantData("");
	else if ((l = dynamic_cast<const ListType *>(t)))
		return new ListConstantData(l);
	else if ((r = dynamic_cast<const RecordType *>(t)))
		return new RecordConstantData(r);
	else
		return nullptr;
}
