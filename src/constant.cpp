#include "constant.hpp"

namespace constant {

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

void BoolConstantData::print(ostream &os) const
{
	os << (value_ ? "true" : "false");
}

void IntConstantData::print(ostream &os) const
{
	os << value_;
}

void StringConstantData::print(ostream &os) const
{
	os << "\"" << value_ << "\"";
}

void RecordConstantData::set(const vector<PrimitiveConstantData *> &v)
{
	if (v.size() != values_.size()) {
		stringstream s;
		s << "wrong number of fields (got " << v.size()
			<< ", expected " << values_.size() << ")";
		throw UnmatchingFieldSignature(s.str());
	}

	auto m = mismatch(values_.begin(), values_.end(), v.begin(),
			[] (const PrimitiveConstantData *c,
				const PrimitiveConstantData *n) {
				return c->type() == n->type();
			});

	if (m.first != values_.end()) {
		stringstream s;
		s << "wrong type for field " << (m.first - values_.begin() + 1)
			<< " (got " << (*m.second)->type()->str() <<
			", expected " << (*m.first)->type()->str() << ")";
		throw UnmatchingFieldSignature(s.str());
	}

	clear();
	values_ = v;
}

void RecordConstantData::set_default()
{
	for (auto it = type_->begin(); it != type_->end(); ++it) {
		RecordField f = (*it);
		values_.push_back(static_cast<PrimitiveConstantData *>(
					create_default_constant(f.type)));
	}
}

void RecordConstantData::clear()
{
	for (auto it = begin(); it != end(); ++it)
		delete (*it);
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

}
