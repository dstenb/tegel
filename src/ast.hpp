#ifndef __AST_H__
#define __AST_H__

using namespace std;

#include "type.hpp"

using namespace type;

namespace ast {

class AST_Node;
class AST_Visitor;

/** Abstract AST node clas
 *
 * An AST_Node object represents a node in the AST of the body section.
 *
 * The nodes are traversed using the visitor pattern (see AST_Visitor)
 */
class AST_Node
{
	public:
		virtual void accept(AST_Visitor &) = 0;
};

class Statements;

class Expression;
class BinaryExpression;
class UnaryExpression;
class And;
class Or;
class Plus;
class Minus;
class Times;
class Constant;
class FunctionCall;
class SymbolRef;

class Statement;
class For;
class If;
class Elif;
class Else;
class Text;
class InlinedExpression;

/**
 *
 */
class Statements : public AST_Node
{

};

/**
 *
 */
class Statement : public AST_Node
{

};

/**
 *
 */
class For : public Statement
{

};

/**
 *
 */
class If : public Statement
{

};

/**
 *
 */
class Else : public Statement
{

};

/** Raw text
 *
 * The Text class holds raw text (in UTF-8 format)
 *
 */
class Text : public Statement
{

};

/**
 *
 */
class InlinedExpression : public Statement
{

};

/**
 *
 */
class Expression : public AST_Node
{
	public:
		virtual const Type *type() = 0;
};

/**
 *
 */
class BinaryExpression : public Expression
{
	public:
		BinaryExpression(Expression *lhs, Expression *rhs)
			: lhs_(lhs), rhs_(rhs) {}

		~BinaryExpression() {
			delete lhs_;
			delete rhs_;
		}
	protected:
		Expression *lhs_;
		Expression *rhs_;

};

/**
 *
 */
class UnaryExpression : public Expression
{

};

/**
 *
 */
class And : public BinaryExpression
{

};

/**
 *
 */
class Or : public BinaryExpression
{

};

/**
 *
 */
class Plus : public BinaryExpression
{

};

/**
 *
 */
class Minus : public BinaryExpression
{

};

/**
 *
 */
class Times : public BinaryExpression
{

};

/**
 *
 */
class Constant : public UnaryExpression
{

};

/**
 *
 */
class FunctionCall : public UnaryExpression
{

};

/**
 *
 */
class SymbolRef : public UnaryExpression
{

};

/**
 *
 */
class AST_Visitor
{
	public:
};

}

#endif
