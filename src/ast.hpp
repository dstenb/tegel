#ifndef __AST_H__
#define __AST_H__

using namespace std;

#include "constant.hpp"
#include "type.hpp"

using namespace constant;
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
	public:
		virtual void accept(AST_Visitor &);
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
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class If : public Statement
{
	public:
		virtual void accept(AST_Visitor &);
};

class Elif : public Statement
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class Else : public Statement
{
	public:
		virtual void accept(AST_Visitor &);
};

/** Raw text
 *
 * The Text class holds raw text (in UTF-8 format)
 *
 */
class Text : public Statement
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class InlinedExpression : public Statement
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class Expression : public AST_Node
{
	public:
		virtual const Type *type() const = 0;
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
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class Or : public BinaryExpression
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class Plus : public BinaryExpression
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class Minus : public BinaryExpression
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class Times : public BinaryExpression
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class Constant : public UnaryExpression
{
	public:
		Constant(ConstantData *d)
			: data_(d) {}

		~Constant() {
			delete data_;
		}

		virtual void accept(AST_Visitor &);
		virtual const Type *type() const { return data_->type(); }
	private:
		ConstantData *data_;
};

/**
 *
 */
class FunctionCall : public UnaryExpression
{
	public:
		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class SymbolRef : public UnaryExpression
{
	public:
		virtual void accept(AST_Visitor &);
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
