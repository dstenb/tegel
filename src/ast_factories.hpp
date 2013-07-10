#ifndef AST_FACTORIES
#define AST_FACTORIES

#include "ast.hpp"

namespace ast_factory {

    using namespace ast;

    struct BoolUnaryFactory
    {
        static Expression *create(Expression *e)
        {
            if (e->type() == TypeFactory::get("bool"))
                return e;
            else if (e->type() == TypeFactory::get("int")) {
                auto zero = new IntConstantData(0);
                return new Not(new Equals(e, new Constant(zero)));
            } else if (e->type() == TypeFactory::get("string")) {
                auto empty = new StringConstantData("");
                return new Not(new StringEquals(e,
                                                new Constant(empty)));
            } else if (e->type()->list()) {
                auto zero = new IntConstantData(0);
                TypeMethod m = e->type()->lookup("size");
                return new GreaterThan(new MethodCall(e, m,  nullptr),
                                       new Constant(zero));
            } else {
                throw InvalidTypeError("Type " + e->type()->str() +
                                       " can't be converted to bool");
            }
        }
    };

    template<class T>
    struct BoolBinaryFactory
    {

        static BinaryExpression *create(Expression *lhs, Expression *rhs)
        {
            return new T(BoolUnaryFactory::create(lhs),
                         BoolUnaryFactory::create(rhs));
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


    struct MinusUnaryFactory
    {
        static BinaryExpression *create(Expression *e)
        {
            if (e->type() == TypeFactory::get("int")) {
                auto zero = new IntConstantData(0);
                return new Minus(new Constant(zero), e);
            }
            throw InvalidTypeError("Can't apply '-' operand on " +
                                   e->type()->str());
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
                if (lhs->type() == TypeFactory::get("bool"))
                    return new BoolEquals(lhs, rhs);
                else if (lhs->type() == TypeFactory::get("int"))
                    return new Equals(lhs, rhs);
                else if (lhs->type() == TypeFactory::get("string"))
                    return new StringEquals(lhs, rhs);
            }
            throw InvalidTypeError("Can't apply '==' operand on " +
                                   lhs->type()->str()  + " and " +
                                   rhs->type()->str());
        }
    };

    struct StringFactory
    {
        static Expression *create(Expression *e)
        {
            if (e->type() == TypeFactory::get("bool")) {
                TypeMethod m = e->type()->lookup("str");
                return new MethodCall(e, m,  nullptr);
            } else if (e->type() == TypeFactory::get("int")) {
                TypeMethod m = e->type()->lookup("str");
                return new MethodCall(e, m,  nullptr);
            } else if (e->type() == TypeFactory::get("string")) {
                return e;
            }
            throw InvalidTypeError("Type " + e->type()->str() +
                                   " can't be converted to string");
        }
    };

}

#endif
