#include "symbol.hpp"

Param *Argument::replace(Param *p)
{
	string id = p->get_id();
	auto it = params_.find(id);

	if (it == params_.end()) {
		throw ParamException("Unknown identifier " + id);
	} else {
		Param *r = it->second;

		if (p->type() != r->type())
		    throw ParamException("Parameter " + id +" must be of type "
				    + type_to_str(r->type()) + " (got " +
				    type_to_str(p->type()) + ")");

		params_[id] = p;
		return r;
	}
}

const Param *Argument::get(const string &s) const
{
	auto it = params_.find(s);
	return (it != params_.end()) ? it->second : nullptr;
}

void Argument::print(ostream &os) const
{
	os << "Argument(" << get_name() << ", " <<
		type_to_str(get_type()) << ")\n";
	for (auto it = params_.begin(); it != params_.end(); ++it) {
		Param *p = it->second;
		os << "\t";
		p->print(os);
		os << "\n";
	}
}

void SymbolTable::add(Symbol *s)
{
	string n = s->get_name();
	auto it = map_.find(n);

	if (it != map_.end())
		throw SymTabAlreadyDefinedError(n);
	map_[n] = s;
}

Symbol *SymbolTable::lookup(const string &s)
{
	auto it = map_.find(s);

	if (it != map_.end())
		return it->second;
	else if (parent_ != nullptr)
		return parent_->lookup(s);
	else
		throw SymTabNoSuchSymbolError(s);
}

void SymbolTable::print(ostream &os) const
{
	for (auto it = map_.begin(); it != map_.end(); it++) {
		it->second->print(os);
		os << endl;
	}
}
