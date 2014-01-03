#ifndef AST_FACTORIES
#define AST_FACTORIES

#include "ast.hpp"

namespace ast_factory {

    using namespace ast;

    /** Converts an expression (if possible) to a bool expression
     *
     */
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
                return new Not(new StringEquals(e, new Constant(empty)));
            } else if (e->type()->list()) {
                TypeMethod m = e->type()->lookup("size");
                return new GreaterThan(new MethodCall(e, m),
                                       new Constant(new IntConstantData(0)));
            } else {
                throw InvalidTypeError("Type " + e->type()->str() +
                                       " can't be converted to bool");
            }
        }
    };

    /** Creates a binary expression of type bool (e.g. and)
     *
     */
    template<class T>
    struct BoolBinaryFactory
    {

        static BinaryExpression *create(Expression *lhs, Expression *rhs)
        {
            return new T(BoolUnaryFactory::create(lhs),
                         BoolUnaryFactory::create(rhs));
        }
    };

    /** Creates a plus expression
     *
     */
    struct PlusBinaryFactory
    {
        static BinaryExpression *create(Expression *lhs, Expression *rhs)
        {
            auto st = TypeFactory::get("string");

            if (lhs->type() == rhs->type()) {
                if (lhs->type() == TypeFactory::get("int"))
                    return new Plus(lhs, rhs);
                else if (lhs->type() == st)
                    return new StringConcat(lhs, rhs);
                else if (lhs->type()->list())
                    return new ListConcat(lhs, rhs);
            } else if (lhs->type() == st) {
                /* Perform a string concatenation if rhs can be converted to a
                 * string */
                try {
                    TypeMethod m = rhs->type()->lookup("str");
                    return new StringConcat(lhs, new ast::MethodCall(rhs, m));
                } catch (const NoSuchMethodError &) {
                    /* The error is handled down below */
                }
            } else if (rhs->type() == st) {
                /* Perform a string concatenation if lhs can be converted to a
                 * string */
                try {
                    TypeMethod m = lhs->type()->lookup("str");
                    return new StringConcat(new ast::MethodCall(lhs, m), rhs);
                } catch (const NoSuchMethodError &) {
                    /* The error is handled down below */
                }
            }
            throw InvalidTypeError("Can't apply '+' operand on " +
                                   lhs->type()->str()  + " and " +
                                   rhs->type()->str());
        }
    };

    /** Creates an unary minus expression
     *
     */
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

    /** Creates a length/size expression
     *
     */
    struct LengthUnaryFactory
    {
        static UnaryExpression *create(Expression *e)
        {
            if (e->type() == TypeFactory::get("string"))
                return new MethodCall(e, e->type()->lookup("length"));
            else if (e->type()->list())
                return new MethodCall(e, e->type()->lookup("size"));

            throw InvalidTypeError("Can't apply '#' operand on " +
                                   e->type()->str());
        }
    };

    /** Creates a minus expression
     *
     */
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

    /** Creates a multiplication expression
     *
     */
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

    /** Creates a < expression
     *
     */
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

    /** Creates a <= expression
     *
     */
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

    /** Creates a > expression
     *
     */
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

    /** Creates a >= expression
     *
     */
    struct GreaterThanOrEqualFactory
    {
        static BinaryExpression *create(Expression *lhs, Expression *rhs)
        {
            if (lhs->type() == rhs->type()) {
                if (lhs->type() == TypeFactory::get("int"))
                    return new GreaterThanOrEqual(lhs, rhs);
                else if (lhs->type() == TypeFactory::get("string"))
                    return new StringGreaterThanOrEqual(lhs, rhs);
            }
            throw InvalidTypeError("Can't apply '>=' operand on " +
                                   lhs->type()->str()  + " and " +
                                   rhs->type()->str());
        }
    };

    /** Creates a == expression
     *
     */
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

    /** Converts an expression (if possible) to a string expression
     *
     */
    struct StringFactory
    {
        static Expression *create(Expression *e)
        {
            if (e->type() == TypeFactory::get("string"))
                return e;

            try {
                return new MethodCall(e, e->type()->lookup("str"));
            } catch (const NoSuchMethodError &) {
                throw InvalidTypeError("Type " + e->type()->str() +
                                       " can't be converted to string");
            }
        }
    };

    /** Creates a ternary if (a ? b : c) expression
     *
     */
    struct TernaryIfFactory
    {
        static TernaryIf *create(Expression *cond,
                                 Expression *if_true, Expression *if_false)
        {
            if (if_true->type() == if_false->type()) {
                return new TernaryIf(BoolUnaryFactory::create(cond),
                                     if_true, if_false);
            }
            throw InvalidTypeError("unmatching types for '?:' operator (got "
                                   + if_true->type()->str() + " and " +
                                   if_false->type()->str() + ")");
        }
    };

    class WrongLambdaSignatureError : public runtime_error
    {
        public:
            WrongLambdaSignatureError(const string &g, const string &e)
                : runtime_error("wrong lambda argument signature (got [" + g
                                + "], expected [" + e + "])") {}
    };

    class WrongFunctionSignatureError : public runtime_error
    {
        public:
            // TODO: add got
            WrongFunctionSignatureError(const string &n, const string &e)
                : runtime_error("wrong arguments for " + n + "(expected ["
                                + e + "])") {}
    };

    class NoSuchFunctionError : public runtime_error
    {
        public:
            NoSuchFunctionError(const string &n)
                : runtime_error("no function named " + n) {}
    };

    /** Creates a function call
     *
     */
    struct FunctionCallFactory
    {
        /** Compares the variables of a lambda expression to a list of expected
         * variables */
        static void check_lambda(const LambdaExpression *lambda,
                                 const vector<const Type *> &expected) {
            vector<const Type *> lambda_vars;

            for (auto p = lambda->variables; p != nullptr; p = p->next)
                lambda_vars.push_back(p->statement->variable()->get_type());

            if (lambda_vars.size() != expected.size()
                    || lambda_vars != expected)
                throw WrongLambdaSignatureError(types_to_str(lambda_vars),
                                                types_to_str(expected));
        }

        static FunctionCall *create(const string &name, FuncArgList *args)
        {
            LambdaExpression *f;
            Expression *e0;

            if (name == "filter") {
                const ListType *list;

                if (!args || !(f = args->get_lambda(0)) ||
                        !(e0 = args->get_expression(1)) ||
                        !(list = e0->type()->list()))
                    throw WrongFunctionSignatureError("map",
                                                      "lambda function, list");
                list = e0->type()->list();
                assert(list);

                check_lambda(f, { list->elem() });

                /* Cast the filter's expression to bool */
                f->expression = BoolUnaryFactory::create(f->expression);

                return new ast::FunctionCall(name, list, args);
            } else if (name == "map") {
                const ListType *list;
                const SingleType *ret_elem;

                if (!args || !(f = args->get_lambda(0)) ||
                        !(e0 = args->get_expression(1)) ||
                        !(list = e0->type()->list()))
                    throw WrongFunctionSignatureError("map",
                                                      "lambda function, list");

                ret_elem = f->expression->type()->single();
                assert(ret_elem);

                check_lambda(f, { list->elem() });

                return new ast::FunctionCall(name,
                                             TypeFactory::get_list(ret_elem), args);
            } else {
                throw NoSuchFunctionError(name);
            }
        }
    };
}

#endif
