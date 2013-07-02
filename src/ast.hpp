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

/** Abstract AST node class
 *
 * An AST_Node object represents a node in the AST of the body section.
 *
 * The nodes are traversed using the visitor pattern (see AST_Visitor)
 */
class AST_Node
{
	public:
		virtual ~AST_Node() {}

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

/** Abstract expression base class
 *
 * An Expression object represents a part of an expression. It has a type
 * associated with it.
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

		Expression *lhs() { return lhs_; }
		Expression *rhs() { return rhs_; }
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
class BinaryBoolExpression : public BinaryExpression
{
	public:
		BinaryBoolExpression(Expression *lhs, Expression *rhs)
			: BinaryExpression(lhs, rhs),
			type_(TypeFactory::get("bool")) {
			assert(lhs->type() == type_);
			assert(rhs->type() == type_);
		}

		virtual void accept(AST_Visitor &) = 0;
		virtual const Type *type() const { return type_; }
	private:
		const Type *type_;
};

/** And class
 *
 * The class only accepts left and right hand sides that are of type bool
 */
class And : public BinaryBoolExpression
{
	public:
		And(Expression *lhs, Expression *rhs)
			: BinaryBoolExpression(lhs, rhs) {}

		virtual void accept(AST_Visitor &);
};

/** Or class
 *
 * The class only accepts left and right hand sides that are of type bool
 */
class Or : public BinaryBoolExpression
{
	public:
		Or(Expression *lhs, Expression *rhs)
			: BinaryBoolExpression(lhs, rhs) {}

		virtual void accept(AST_Visitor &);
};

/**
 *
 */
class BinaryIntExpression : public BinaryExpression
{
	public:
		BinaryIntExpression(Expression *lhs, Expression *rhs)
			: BinaryExpression(lhs, rhs),
			type_(TypeFactory::get("int")) {
			assert(lhs->type() == type_);
			assert(rhs->type() == type_);
		}

		virtual void accept(AST_Visitor &) = 0;
		virtual const Type *type() const { return type_; }
	private:
		const Type *type_;
};

/** Plus class
 *
 * Represents integer addition
 */
class Plus : public BinaryIntExpression
{
	public:
		Plus(Expression *lhs, Expression *rhs)
			: BinaryIntExpression(lhs, rhs) {}

		virtual void accept(AST_Visitor &);
};

/** Minus class
 *
 * Represents integer subtraction
 */
class Minus : public BinaryIntExpression
{
	public:
		Minus(Expression *lhs, Expression *rhs)
			: BinaryIntExpression(lhs, rhs) {}
		virtual void accept(AST_Visitor &);
};

/** Times class
 *
 * Represents integer multiplication
 */
class Times : public BinaryIntExpression
{
	public:
		Times(Expression *lhs, Expression *rhs)
			: BinaryIntExpression(lhs, rhs) {}

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

		ConstantData *data() { return data_; }

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

/** Statements class
 *
 * Statements represents a list of statements. A Statements object holds a
 * pointer to the previous statements object.
 */
class Statements : public AST_Node
{
	public:
		Statements(Statement *s, Statements *p = nullptr)
			: statement_(s), prev_(p) {}

		~Statements() {
			delete statement_;
			if (prev_)
				delete prev_;
		}
		// TODO: add destructor

		virtual void accept(AST_Visitor &);

		Statement *statement() { return statement_; }
		Statements *prev() { return prev_; }
	private:
		Statement *statement_;
		Statements *prev_;
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

/**
 *
 */
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
		Text(const string &s)
			: text_(s) {}

		virtual void accept(AST_Visitor &);

		string text() const { return text_; }
	private:
		string text_;
};

/**
 *
 */
class InlinedExpression : public Statement
{
	public:
		InlinedExpression(Expression *e) {

		}

		~InlinedExpression() {
			delete  expression_;
		}

		Expression *expression() { return expression_; }

		virtual void accept(AST_Visitor &);
	private:
		Expression *expression_;
};

/**
 *
 */
class AST_Visitor
{
	public:
		virtual void visit(And *) = 0;
		virtual void visit(Or *) = 0;
		virtual void visit(Plus *) = 0;
		virtual void visit(Minus *) = 0;
		virtual void visit(Times *) = 0;
		virtual void visit(Constant *) = 0;
		virtual void visit(FunctionCall *) = 0;
		virtual void visit(SymbolRef *) = 0;

		virtual void visit(Statements *) = 0;

		virtual void visit(For *) = 0;
		virtual void visit(If *) = 0;
		virtual void visit(Elif *) = 0;
		virtual void visit(Else *) = 0;
		virtual void visit(Text *) = 0;
		virtual void visit(InlinedExpression *) = 0;
};

class AST_Printer : public AST_Visitor
{
	public:
		virtual void visit(Statements *p) {
			print_ws();
			cerr << "Statements\n";
			indent++;

			if (p->prev())
				p->prev()->accept(*this);
			p->statement()->accept(*this);

			indent--;
		}

		virtual void visit(And *p) {
			binary("And", p);
		}

		virtual void visit(Or *p) {
			binary("Or", p);
		}

		virtual void visit(Plus *p) {
			binary("+", p);
		}

		virtual void visit(Minus *p) {
			binary("-", p);
		}

		virtual void visit(Times *p) {
			binary("*", p);
		}

		virtual void visit(Constant *p) {
			print_ws();
			cerr << "Constant(";
			p->data()->print(cerr);
			cerr << ")\n";
		}

		virtual void visit(FunctionCall *p) {
			print_ws();
			cerr << "FunctionCall\n";
		}

		virtual void visit(SymbolRef *p) {
			print_ws();
			cerr << "SymbolRef\n";
		}

		virtual void visit(For *p) {
			print_ws();
			cerr << "For\n";
		}

		virtual void visit(If *p) {
			print_ws();
			cerr << "If\n";
		}

		virtual void visit(Elif *p) {
			print_ws();
			cerr << "Elif\n";
		}

		virtual void visit(Else *p)  {
			print_ws();
			cerr << "Else\n";
		}

		virtual void visit(Text *p) {
			print_ws();
			cerr << "Text('" << p->text() << "')\n";
		}

		virtual void visit(InlinedExpression *p) {
			print_ws();
			cerr << "InlinedExpression\n";
		}

	private:
		void binary(const string &s, BinaryExpression *e)
		{
			print_ws();
			cerr << s << "\n";
			indent++;
			e->lhs()->accept(*this);
			e->rhs()->accept(*this);
			indent--;
		}

		int indent = 0;
		void print_ws() {
			for (int i = 0; i < indent; i++)
				cerr << " ";
		}
};

}

#endif
