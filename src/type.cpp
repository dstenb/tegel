#include "type.hpp"

string type_to_str(Type t) {
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
		default:
			return "";
	}
}
