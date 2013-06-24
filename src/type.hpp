#ifndef __TYPE_H__
#define __TYPE_H__

#include <string>

using std::string;

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

#endif
