#include "py_backend.hpp"

namespace py_backend {

	ostream &PyWriter::indent()
	{
		for (unsigned i = 0; i < indentation_; i++)
			os_ << "    ";
		return os_;
	}

	ostream &PyWriter::unindent()
	{
		return os_;
	}

	void PyWriter::indent_dec()
	{
		if (indentation_ > 0)
			indentation_--;
	}

	void PyWriter::indent_inc()
	{
		indentation_++;
	}


	void PyHeader::generate(const vector<symbol::Argument *> &args)
	{
		indent() << "import getopt\n";
		indent() << "import sys\n";
		/* TODO */
	}

	void PyBody::generate(ast::Statements *body)
	{
		indent() << "def generate(_args):\n";
		indent_inc();
		body->accept(*this);
		indent_dec();
	}

	void PyBody::visit(ast::Statements *p)
	{
		p->statement()->accept(*this);
		if (p->next())
			p->next()->accept(*this);
	}

	void PyBody::visit(ast::And *p)
	{
		binary("and", p);
	}

	void PyBody::visit(ast::Or *p)
	{
		binary("or", p);
	}

	void PyBody::visit(ast::Not *p)
	{
		unindent() << "not ";
		p->expression()->accept(*this);
	}

	void PyBody::visit(ast::BoolEquals *p)
	{
		binary("==", p);
	}

	void PyBody::visit(ast::LessThan *p)
	{
		binary("<", p);
	}

	void PyBody::visit(ast::LessThanOrEqual *p)
	{
		binary("<=", p);
	}

	void PyBody::visit(ast::GreaterThan *p)
	{
		binary(">", p);
	}

	void PyBody::visit(ast::GreaterThanOrEqual *p)
	{
		binary(">=", p);
	}

	void PyBody::visit(ast::Equals *p)
	{
		binary("==", p);
	}

	void PyBody::visit(ast::Plus *p)
	{
		binary("+", p);
	}

	void PyBody::visit(ast::Minus *p)
	{
		binary("-", p);
	}

	void PyBody::visit(ast::Times *p)
	{
		binary("*", p);
	}

	void PyBody::visit(ast::StringLessThan *p)
	{
		binary("<", p);
	}

	void PyBody::visit(ast::StringLessThanOrEqual *p)
	{
		binary("<=", p);
	}

	void PyBody::visit(ast::StringGreaterThan *p)
	{
		binary(">", p);
	}

	void PyBody::visit(ast::StringGreaterThanOrEqual *p)
	{
		binary(">=", p);
	}

	void PyBody::visit(ast::StringEquals *p)
	{
		binary("==", p);
	}

	void PyBody::visit(ast::StringRepeat *p)
	{
		binary("*", p);
	}

	void PyBody::visit(ast::StringConcat *p)
	{
		binary("+", p);
	}

	void PyBody::visit(ast::ListConcat *p)
	{
		binary("+", p);
	}

	void PyBody::visit(ast::Constant *p)
	{
		if (p->data()->type() == TypeFactory::get("bool")) {
			auto b = (BoolConstantData *)p->data();
			unindent() << (b ? "True" : "False");
		} else if (p->data()->type() == TypeFactory::get("string")) {
			auto s = (StringConstantData *)p->data();
			unindent() << "\"" << Escaper()(s->value()) << "\"";
		} else {
			p->data()->print(cerr);
		}
	}

	void PyBody::visit(ast::MethodCall *p)
	{
		/* TODO */
		auto t = p->expression()->type();
		auto m = p->method();

		if (t == type::TypeFactory::get("bool")) {
			if (m.name() == "str") {
				unindent() << "str(";
				p->expression()->accept(*this);
				unindent() << ")";
			}
		} else if (t == type::TypeFactory::get("int")) {
			if (m.name() == "downto") {
				unindent() << "list(reversed(range(";
				p->expression()->accept(*this);
				unindent() << ", ";
				p->arguments()->expression->accept(*this);
				unindent() << ")))";
			}
			if (m.name() == "str") {
				unindent() << "str(";
				p->expression()->accept(*this);
				unindent() << ")";
			}
			if (m.name() == "upto") {
				unindent() << "list(range(";
				p->expression()->accept(*this);
				unindent() << ", ";
				p->arguments()->expression->accept(*this);
				unindent() << ")";
			}
		} else if (t == type::TypeFactory::get("string")) {
			if (m.name() == "length") {
				unindent() << "len(";
				p->expression()->accept(*this);
				unindent() << ")";
			} else if (m.name() == "lower") {
				p->expression()->accept(*this);
				unindent() << ".lower()";
			} else if (m.name() == "title") {
				p->expression()->accept(*this);
				unindent() << ".title()";
			} else if (m.name() == "upper") {
				p->expression()->accept(*this);
				unindent() << ".upper()";
			}
		} else if (t->list()) {

		} else if (t->record()) {

		}

		// p->expression()->accept(*this);
		// unindent() << "." << p->method().name() << "(";
		// for (auto e = p->arguments(); e != nullptr;
		// 		e = e->next) {
		// 	e->expression->accept(*this);
		// 	if (e->next)
		// 		unindent() << ", ";
		// }
		// unindent() << ")";
	}

	void PyBody::visit(ast::SymbolRef *p)
	{
		/* TODO */
		unindent() << p->symbol()->get_name();
	}

	void PyBody::visit(ast::List *p)
	{
		unindent() << "[";
		for (auto e = p->elements(); e != nullptr;
				e = e->next) {
			e->expression->accept(*this);

			if (e->next)
				unindent() << ", ";
		}
		unindent() << "]";
	}

	void PyBody::visit(ast::Conditional *p)
	{
		p->if_node()->accept(*this);
		if (p->elif_nodes())
			p->elif_nodes()->accept(*this);
		if (p->else_node())
			p->else_node()->accept(*this);
	}

	void PyBody::visit(ast::ForEach *p)
	{
		if (p->statements()) {
			indent() << "for ";
			unindent() << p->variable()->get_name();
			unindent() << " in ";
			p->expression()->accept(*this);
			unindent() << ":\n";
			indent_inc();
			p->statements()->accept(*this);
			indent_dec();
		}
	}

	void PyBody::visit(ast::If *p)
	{
		indent() << "if ";
		p->condition()->accept(*this);
		unindent() << ":\n";
		indent_inc();
		if (p->statements())
			p->statements()->accept(*this);
		indent_dec();
	}

	void PyBody::visit(ast::Elif *p)
	{
		indent() << "elif ";
		p->condition()->accept(*this);
		unindent() << ":\n";
		indent_inc();
		if (p->statements())
			p->statements()->accept(*this);
		indent_dec();
		if (p->next())
			p->next()->accept(*this);
	}

	void PyBody::visit(ast::Else *p)
	{
		if (p->statements()) {
			indent() << "else:\n";
			indent_inc();
			p->statements()->accept(*this);
			indent_dec();
		}
	}

	void PyBody::visit(ast::Text *p)
	{
		indent() << "print('" << Escaper()(p->text()) << "')\n";
	}

	void PyBody::visit(ast::InlinedExpression *p)
	{
		indent() << "print(";
		p->expression()->accept(*this);
		unindent() << ")\n";
	}

	void PyBody::binary(const string &s, ast::BinaryExpression *e)
	{
		unindent() << "(";
		e->lhs()->accept(*this);
		unindent() << " " << s << " ";
		e->rhs()->accept(*this);
		unindent() << ")";
	}

	void PyUsage::generate(const vector<symbol::Argument *> &args) {
		indent() << "def usage(cmd):\n";
		indent_inc();
		indent() << "print(\"Usage: %s [OPTIONS...]\" % cmd)\n";
		for (auto it = args.begin(); it != args.end(); ++it) {
			indent() << "print(\"";

			/* Print command line strings */
			/* TODO */

			/* Print argument information */
			auto p = (*it)->get("info");
			auto s = (StringConstantData *)p->get();
			unindent() << Escaper()(s->value());
			unindent() << "\")\n";
		}
		indent_dec();
	}

	void PyMain::generate(const vector<symbol::Argument *> &args)
	{
		indent() << "def main(argv=None):\n";
		indent_inc();
		indent() << "if argv is None:\n";
		indent() << "   argv = sys.argv\n\n";

		generate_arg_dict(args);
		indent() << "\n";

		generate_arg_list(args);
		indent() << "\n";

		indent() << "if __name__ == \"__main__\":\n";
		indent() << "    main()\n";
	}

	void PyMain::generate_arg_dict(const vector<symbol::Argument *> &args)
	{
		indent() << "args = {\n";
		indent_inc();
		for (auto it = args.begin(); it != args.end(); ++it) {
			indent() << "\"" << (*it)->get_name()
				<< "\": {\n";
			indent_inc();
			/* TODO */
			indent_dec();
			indent() << "}\n";
		}
		indent_dec();
		indent() << "}\n";
	}

	void PyMain::generate_arg_list(const vector<symbol::Argument *> &args)
	{
		indent() << "args_order = [\n";
		indent_inc();
		for (auto it = args.begin(); it != args.end(); ++it) {
			indent() << "\"" << (*it)->get_name()
				<< "\", \n";
		}
		indent_dec();
		indent() << "]\n";
	}

	void PyBackend::generate(ostream &os,
			const vector<symbol::Argument *> &args,
			ast::Statements *body)
	{
		if (body) {
			PyHeader h(os);
			PyBody b(os);
			PyUsage u(os);
			PyMain m(os);

			h.generate(args);
			os << "\n";
			b.generate(body);
			os << "\n";
			u.generate(args);
			os << "\n";
			m.generate(args);
		}
	}
}
