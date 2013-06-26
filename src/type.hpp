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
	StringListType
};

string type_to_str(Type t);

class InvalidTypeError : public runtime_error
{
	public:
		InvalidTypeError(Type g, Type e)
			: runtime_error("Got " + type_to_str(g) +
					", expected " + type_to_str(e)) {}
};

#endif
