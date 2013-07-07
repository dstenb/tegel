#ifndef AST_FACTORIES
#define AST_FACTORIES

#include "ast.hpp"

namespace ast_factory {

using namespace ast;

template<class T>
struct BoolBinaryFactory
{

	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() != TypeFactory::get("bool"))
			throw DifferentTypesError(lhs->type(),
					TypeFactory::get("bool"));
		if (rhs->type() != TypeFactory::get("bool"))
			throw DifferentTypesError(rhs->type(),
					TypeFactory::get("bool"));
		return new T(lhs, rhs);
	}
};

struct PlusBinaryFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new Plus(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new StringConcat(lhs, rhs);
			else if (lhs->type()->list())
				return new ListConcat(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '+' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct MinusBinaryFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() == rhs->type() &&
				lhs->type() == TypeFactory::get("int"))
			return new Minus(lhs, rhs);
		throw InvalidTypeError("Can't apply '-' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct TimesBinaryFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		const Type *integer = TypeFactory::get("int");
		const Type *string = TypeFactory::get("string");

		if (lhs->type() == string && rhs->type() == integer)
			return new StringRepeat(lhs, rhs);
		else if (lhs->type() == integer && rhs->type() == string)
			return new StringRepeat(rhs, lhs);
		else if (lhs->type() == integer && lhs->type() == rhs->type())
			return new Times(lhs, rhs);
		throw InvalidTypeError("Can't apply '*' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct LessThanFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new LessThan(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new StringLessThan(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '<' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct LessThanOrEqualFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new LessThanOrEqual(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new StringLessThanOrEqual(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '<=' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct GreaterThanFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new GreaterThan(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new StringGreaterThan(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '>' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct GreaterThanOrEqualFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new LessThan(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new StringLessThan(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '>=' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct EqualsFactory
{
	static BinaryExpression *create(Expression *lhs, Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new Equals(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new StringEquals(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '>=' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

}

#endif
