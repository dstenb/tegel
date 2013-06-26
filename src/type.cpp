#include "type.hpp"

string type_to_str(Type t)
{
	switch (t) {
		case BoolType:
			return "bool";
		case BoolListType:
			return "bool[]";
		case IntType:
			return "int";
		case IntListType:
			return "int[]";
		case StringType:
			return "string";
		case StringListType:
			return "string[]";
		case EmptyList:
			return "[]";
		default:
			return "";
	}
}

Type type_list_to_scalar(Type t)
{
	if (t == BoolListType)
		return BoolType;
	else if (t == IntListType)
		return IntType;
	else if (t == StringListType)
		return StringType;
	else
		throw InvalidTypeError("Expected a list type (got " +
				type_to_str(t) + ")");
}

Type type_scalar_to_list(Type t)
{
	if (t == BoolType)
		return BoolListType;
	else if (t == IntType)
		return IntListType;
	else if (t == StringType)
		return StringListType;
	else
		throw InvalidTypeError("Expected a scalar type (got " +
				type_to_str(t) + ")");
}

bool type_is_list(Type t)
{
	return (t == BoolListType || t == IntListType || t == StringListType);
}
