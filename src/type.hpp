#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdexcept>
#include <string>

using std::string;
using std::runtime_error;

/* I might change this to something more flexible in the future */
enum Type
{
	BoolType,
	BoolListType,
	IntType,
	IntListType,
	StringType,
	StringListType,
	EmptyList
};

string type_to_str(Type t);
bool type_is_list(Type t);
Type type_list_to_scalar(Type t);
Type type_scalar_to_list(Type t);

class DifferentTypesError : public runtime_error
{
	public:
		DifferentTypesError(Type g, Type e)
			: runtime_error("Got " + type_to_str(g) +
					", expected " + type_to_str(e)) {}
};

class InvalidTypeError : public runtime_error
{
	public:
		InvalidTypeError(const string &what)
			: runtime_error(what) {}
};

#endif
