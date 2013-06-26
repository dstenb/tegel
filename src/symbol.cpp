#include "symbol.hpp"

void Argument::print(ostream &os) const {
	os << "Argument(" << get_name() << ", " <<
		type_to_str(get_type()) << ")";
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
