#ifndef AST_FACTORIES
#define AST_FACTORIES

#include "ast.hpp"

namespace ast_factory {

template<class T>
struct BoolBinaryFactory
{

	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
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
	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new ast::Plus(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new ast::StringConcat(lhs, rhs);
			else if (lhs->type()->list())
				return new ast::ListConcat(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '+' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct MinusBinaryFactory
{
	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
	{
		if (lhs->type() == rhs->type() &&
				lhs->type() == TypeFactory::get("int"))
			return new ast::Minus(lhs, rhs);
		throw InvalidTypeError("Can't apply '-' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct TimesBinaryFactory
{
	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
	{
		const Type *integer = TypeFactory::get("int");
		const Type *string = TypeFactory::get("string");

		if (lhs->type() == string && rhs->type() == integer)
			return new ast::StringRepeat(lhs, rhs);
		else if (lhs->type() == integer && rhs->type() == string)
			return new ast::StringRepeat(rhs, lhs);
		else if (lhs->type() == integer && lhs->type() == rhs->type())
			return new ast::Times(lhs, rhs);
		throw InvalidTypeError("Can't apply '*' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct LessThanFactory
{
	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new ast::LT(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new ast::StringLT(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '<' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct LessThanOrEqualFactory
{
	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new ast::LE(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new ast::StringLE(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '<=' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct GreaterThanFactory
{
	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new ast::GT(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new ast::StringGT(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '>' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

struct GreaterThanOrEqualFactory
{
	static ast::BinaryExpression *create(ast::Expression *lhs,
			ast::Expression *rhs)
	{
		if (lhs->type() == rhs->type()) {
			if (lhs->type() == TypeFactory::get("int"))
				return new ast::LT(lhs, rhs);
			else if (lhs->type() == TypeFactory::get("string"))
				return new ast::StringLT(lhs, rhs);
		}
		throw InvalidTypeError("Can't apply '>=' operand on " +
				lhs->type()->str()  + " and " +
				rhs->type()->str());
	}
};

}

#endif
