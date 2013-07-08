#ifndef __PYTHON_BACKEND_H__
#define __PYTHON_BACKEND_H__

#include <string>

using namespace std;

#include "backend.hpp"
#include "common.hpp"
#include "type.hpp"

namespace py_backend {

struct PyUtils
{
	static string constant_to_str(const ConstantData *c) {
		if (c->type() == TypeFactory::get("bool")) {
			auto b = (const BoolConstantData *)c;
			return (b->value() ? "True" : "False");
		} else if (c->type() == TypeFactory::get("int")) {
			auto i = (const IntConstantData *)c;
			return to_string(i->value());
		} else if (c->type() == TypeFactory::get("string")) {
			auto s = (const StringConstantData *)c;
			return "\"" + Escaper()(s->value()) + "\"";
		} else if (c->type()->list()) {
			/* TODO */
			return "[ 1, 2, 3 ]";
		} else if (c->type()->record()) {
			/* TODO */
			return "book{1, \"hej\"}";
		}
	}
};

/** PyWriter class
 *
 * PyWriter provides python indentation (i.e. levels of 4 spaces) to an ostream
 *
 */
class PyWriter
{
	public:
		PyWriter(ostream &os, unsigned indentation = 0)
			: os_(os), indentation_(indentation) {}

		virtual ~PyWriter() {}
	protected:
		ostream &indent();
		ostream &unindent();

		void indent_dec();
		void indent_inc();
	private:
		ostream &os_;
		unsigned indentation_;
};

class PyHeader : public PyWriter
{
	public:
		PyHeader(ostream &os)
			: PyWriter(os, 0) {}

		void generate(const vector<symbol::Argument *> &);
};

class PyBody : public PyWriter, public ast::AST_Visitor
{
	public:
		PyBody(ostream &os)
			: PyWriter(os, 0) {}

		void generate(ast::Statements *body);

		virtual void visit(ast::And *);
		virtual void visit(ast::Or *);
		virtual void visit(ast::Not *);
		virtual void visit(ast::BoolEquals *);
		virtual void visit(ast::LessThan *);
		virtual void visit(ast::LessThanOrEqual *);
		virtual void visit(ast::GreaterThan *);
		virtual void visit(ast::GreaterThanOrEqual *);
		virtual void visit(ast::Equals *);
		virtual void visit(ast::Plus *);
		virtual void visit(ast::Minus *);
		virtual void visit(ast::Times *);
		virtual void visit(ast::StringLessThan *);
		virtual void visit(ast::StringLessThanOrEqual *);
		virtual void visit(ast::StringGreaterThan *);
		virtual void visit(ast::StringGreaterThanOrEqual *);
		virtual void visit(ast::StringEquals *);
		virtual void visit(ast::StringRepeat *);
		virtual void visit(ast::StringConcat *);
		virtual void visit(ast::ListConcat *);
		virtual void visit(ast::Constant *);
		virtual void visit(ast::MethodCall *);
		virtual void visit(ast::SymbolRef *);
		virtual void visit(ast::List *);
		virtual void visit(ast::Statements *);
		virtual void visit(ast::Conditional *);
		virtual void visit(ast::ForEach *);
		virtual void visit(ast::If *);
		virtual void visit(ast::Elif *);
		virtual void visit(ast::Else *);
		virtual void visit(ast::Text *);
		virtual void visit(ast::InlinedExpression *);
	private:
		void binary(const string &s, ast::BinaryExpression *e);
};

class PyUsage : public PyWriter
{
	public:
		PyUsage(ostream &os)
			: PyWriter(os, 0) {}

		void generate(const vector<symbol::Argument *> &);
};

class PyMain : public PyWriter
{
	public:
		PyMain(ostream &os)
			: PyWriter(os, 0) {}

		void generate(const vector<symbol::Argument *> &);
	private:
		void generate_arg_dict(const vector<symbol::Argument *> &);
		void generate_arg_list(const vector<symbol::Argument *> &);
};

class PyBackend : public Backend
{
	public:
		void generate(ostream &, const vector<symbol::Argument *> &,
				ast::Statements *);
};

}

#endif
