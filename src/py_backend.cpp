#include "py_backend.hpp"

namespace py_backend {

	string PyUtils::constant_to_str(const ConstantData *c)
	{
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
			auto l = (ListConstantData *)c;

			stringstream sstr;
			sstr << "[";

			auto v = l->values();
			auto it = v.begin();

			while (it != v.end()) {
				sstr << constant_to_str(*it);
				if (++it != v.end())
					sstr << ", ";
			}

			sstr << "]";

			return sstr.str();
		} else if (c->type()->record()) {
			auto r = (RecordConstantData *)c;

			stringstream sstr;
			sstr << record_name(r->type()) << "(";

			auto v = r->values();
			auto it = v.begin();

			while (it != v.end()) {
				sstr << constant_to_str(*it);
				if (++it != v.end())
					sstr << ", ";
			}

			sstr << ")";

			return sstr.str();
		} else {
			return "";
		}
	}

	string PyUtils::record_name(const RecordType *r)
	{
		return "tuple_" + r->str();
	}

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
		unindent() << "import getopt\n";
		unindent() << "import sys\n";
		unindent() << "from collections import namedtuple\n\n";

		generate_records(args);
	}

	void PyHeader::generate_records(const vector<symbol::Argument *> &args)
	{
		for (symbol::Argument *a : args) {
			auto t = a->get_type();
			const RecordType *r;

			if ((r = t->record())) {
				string name = PyUtils::record_name(r);
				unindent() << name << " = namedtuple(\""
					<< name << "\", [";

				auto it = r->begin();

				while (it != r->end()) {
					unindent() << "\"" << (*it).name
						<< "\"";
					if (++it != r->end())
						unindent() << ", ";
				}

				unindent() << "])\n";
			}
		}
	}

	void PyBody::generate(ast::Statements *body)
	{
		indent() << "def generate(_args, _file):\n";
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
		unindent() << PyUtils::constant_to_str(p->data());
	}

	void PyBody::visit(ast::MethodCall *p)
	{
		/* TODO */
		auto t = p->expression()->type();
		auto m = p->method();

		if (t == type::TypeFactory::get("bool")) {
			if (m.name() == "str") {
				unindent() << "\"true\" if ";
				p->expression()->accept(*this);
				unindent() << " else \"false\"";
			}
		} else if (t == type::TypeFactory::get("int")) {
			if (m.name() == "downto") {
				unindent() << "list(reversed(range(";
				p->arguments()->expression->accept(*this);
				unindent() << ", ";
				p->expression()->accept(*this);
				unindent() << "+ 1)))";
			} else if (m.name() == "str") {
				unindent() << "str(";
				p->expression()->accept(*this);
				unindent() << ")";
			} else if (m.name() == "upto") {
				unindent() << "list(range(";
				p->expression()->accept(*this);
				unindent() << ", ";
				p->arguments()->expression->accept(*this);
				unindent() << "+ 1))";
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
			auto tl = t->list();

			if (m.name() == "size") {
				unindent() << "len(";
				p->expression()->accept(*this);
				unindent() << ")";
			} else if (m.name() == "sort") {
				if (tl->elem()->primitive()) {
					unindent() << "sorted(";
					p->expression()->accept(*this);
					unindent() << ", reverse=not ";
					p->arguments()->expression->accept(*this);
					unindent() << ")";
				} else if (tl->elem()->record()) {
					/* TODO */
				}
			}
		} else if (t->record()) {

		} else {

		}
	}

	void PyBody::visit(ast::SymbolRef *p)
	{
		/* TODO */

		symbol::Argument *a;
		symbol::Variable *v;

		if (dynamic_cast<symbol::Argument *>(p->symbol())) {
		    unindent() << "_args[\"" << p->symbol()->get_name()
			    << "\"][\"value\"]";
		} else if (dynamic_cast<symbol::Variable *>(p->symbol())) {
		    unindent() << p->symbol()->get_name();
		}
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
		indent() << "_file.write('" << Escaper()(p->text()) << "')\n";
	}

	void PyBody::visit(ast::InlinedExpression *p)
	{
		indent() << "_file.write(";
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

	void PyUsage::generate(const vector<symbol::Argument *> &args)
	{
		indent() << "def usage(cmd):\n";
		indent_inc();
		indent() << "_file.write(\"Usage: %s [OPTIONS...]\" % cmd)\n";
		for (auto it = args.begin(); it != args.end(); ++it) {
			indent() << "_file.write(\"";

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
		unindent() << "\n";

		generate_arg_list(args);
		unindent() << "\n";

		indent() << "generate(args, sys.stdout)\n\n";

		generate_opts(args);

		indent_dec();

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

			auto p = (*it)->get("default");
			indent() << "\"value\": ";
			unindent() << PyUtils::constant_to_str(p->get());
			unindent() << ",\n";

			indent_dec();
			indent() << "},\n";
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

	void PyMain::generate_opts(const vector<symbol::Argument *> &args)
	{

	}

	void PyBackend::generate(ostream &os,
			const vector<symbol::Argument *> &args,
			ast::Statements *body)
	{
		if (body) {
			check_cmd(args);

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

	void PyBackend::check_cmd(const vector<symbol::Argument *> &args)
	{
		vector<string> reserved = { "h", "o" };
		vector<string> handled = reserved;

		for (auto a : args) {
			auto p = a->get("cmd");
			auto l = (ListConstantData *)p->get();

			/* TODO */
			for (auto e : l->values()) {
				auto s = (StringConstantData *)e;
				string c = s->value();

				/* TODO: check format */

				if (find(reserved.begin(), reserved.end(), c)
						!= reserved.end()) {
					throw BackendException(
							"Reserved command "
							"line name: " + c);
				}
				if (find(handled.begin(), handled.end(),
							c) != handled.end()) {
					throw BackendException(
							"Multiple command "
							"line arguments named "
							+ c);
				}

				handled.push_back(c);
			}
		}
	}
}
