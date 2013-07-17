#include "py_backend.hpp"

namespace py_backend
{

    /** Converts a TeGeL constant to the corresponding constant in Python
     *
     */
    class PyConstToStream : public ConstantDataVisitor
    {
        public:
            PyConstToStream(ostream &os)
                : os_(os) {}

            virtual void visit(const BoolConstantData *p) {
                os_ << (p->value() ? "True" : "False");
            }

            virtual void visit(const IntConstantData *p) {
                os_ << to_string(p->value());
            }

            virtual void visit(const StringConstantData *p) {
                os_ << "\"" << Escaper()(p->value()) << "\"";
            }

            virtual void visit(const ListConstantData *p) {
                auto it = p->begin();
                auto end = p->end();

                os_ << "[";
                while (it != end) {
                    (*it)->accept(*this);
                    if (++it != end)
                        os_ << ", ";
                }
                os_ << "]";
            }

            virtual void visit(const RecordConstantData *p) {
                auto it = p->begin();
                auto end = p->end();

                os_ << PyUtils::record_name(p->type()) << "(";
                while (it != end) {
                    (*it)->accept(*this);
                    if (++it != end)
                        os_ << ", ";
                }
                os_ << ")";
            }
        protected:
            ostream &os_;
    };

    /** Outputs a colon delimited list of field type information (used for the
     * command-line error message)
     *
     */
    class PyRecordColonDelim : public TypeVisitor
    {
        public:
            PyRecordColonDelim(ostream &os)
                : os_(os) {}

            virtual void visit(const RecordType *p) {
                auto it = p->begin();
                while (it != p->end()) {
                    (*it).type->accept(*this);
                    if (++it != p->end())
                        os_ << ":";
                }
            }

            virtual void visit(const BoolType *) {
                os_ << "[y/n]";
            }

            virtual void visit(const IntType *) {
                os_ << "int";
            }

            virtual void visit(const StringType *) {
                os_ << "string";
            }

            virtual void visit(const ListType *) { }
        private:
            ostream &os_;
    };

    /** Outputs a comma delimited list of casts to the proper types for
     * each of the record's fields
     *
     */
    class PyRecordCastList : public TypeVisitor
    {
        public:
            PyRecordCastList(ostream &os)
                : os_(os) {}

            virtual void visit(const RecordType *p) {
                auto it = p->begin();
                while (it != p->end()) {
                    (*it).type->accept(*this);
                    i++;
                    if (++it != p->end())
                        os_ << ", ";
                }
            }

            virtual void visit(const BoolType *) {
                os_ << "parse_bool(l[" << i << "])";
            }

            virtual void visit(const IntType *) {
                os_ << "int(l[" << i << "])";
            }

            virtual void visit(const StringType *) {
                os_ << "l[" << i << "]";
            }

            virtual void visit(const ListType *) { }
        private:
            int i = 0;
            ostream &os_;
    };

    /** Helper function for PyConstToStream
     *
     */
    ostream &PyUtils::constant_to_stream(ostream &os, const ConstantData *c)
    {
        PyConstToStream cs(os);
        c->accept(cs);
        return os;
    }

    /** Returns the record name (the name used for the named tuples)
     *
     */
    string PyUtils::record_name(const RecordType *r)
    {
        return "tuple_" + r->str();
    }

    bool PyUtils::is_short_cmd(const string &s)
    {
        return (s.length() == 2 && s[0] == '-' && isalpha(s[1]));
    }

    bool PyUtils::is_long_cmd(const string &s)
    {
        if (s.length() > 2) {
            if (!s[0] == '-' || !s[1] == '-')
                return false;
            auto it = s.begin();
            advance(it, 2);

            for ( ; it != s.end(); ++it)
                if (!isalpha((*it)))
                    return false;
            return true;
        }
        return false;
    }

    bool PyUtils::valid_cmd_format(const string &s)
    {
        return is_short_cmd(s) || is_long_cmd(s);
    }

    /** Outputs an argparse line from an argument
     *
     */
    void PyUtils::generate_opt(ostream &os, symbol::Argument *a)
    {
        /* Get the command line identifier */
        auto p = a->get("cmd");
        auto cd = static_cast<const StringConstantData *>(p->get());
        auto cs = cd->value();

        os << "parser.add_argument(\"" << cs << "\"";

        /* Get the help (information) string */
        p = a->get("info");
        auto id = static_cast<const StringConstantData *>(p->get());
        auto is = Escaper()(id->value());

        /* Get the default value */
        auto dd = a->get("default")->get();

        const Type *t = dd->type();

        if (t == TypeFactory::get("bool")) {
            os << ", type=parse_bool";
        } else if (t == TypeFactory::get("int")) {
            os << ", type=int";
        } else if (t == TypeFactory::get("string")) {
            os << ", type=str";
        } else if (t->list()) {
            auto e = t->list()->elem();

            if (e == TypeFactory::get("bool")) {
                os << ", nargs=\"+\", type=parse_bool";
            } else if (e == TypeFactory::get("int")) {
                os << ", nargs=\"+\", type=int";
            } else if (e == TypeFactory::get("string")) {
                os << ", nargs=\"+\", type=str";
            } else if (e->record()) {
                os << ", nargs=\"+\", type=parse_"
                   << PyUtils::record_name(e->record());
            }
        } else if (t->record()) {
            os << ", type=parse_"
               << PyUtils::record_name(t->record());
        }

        PyConstToStream pcs(os);
        os << ", default=";
        dd->accept(pcs);

        os << ", help=\"" << is << "\"";
        os << ", dest=\"" << a->get_name() << "\"";
        os << ")\n";
    }

    void PyUtils::check_cmd(const vector<symbol::Argument *> &args,
                            const vector<string> &reserved)
    {
        vector<string> handled = reserved;

        for (auto a : args) {
            auto p = a->get("cmd");
            auto s = (StringConstantData *)p->get();

            string c = s->value();

            if (c.size() == 0) {
                throw BackendException(
                    "no command line name given for argument '" +
                    a->get_name() + "'");
            }

            if (!PyUtils::valid_cmd_format(c)) {
                throw BackendException(
                    "invalid command line name: '" + c +
                    "' given for argument '" + a->get_name() + "' (valid "
                    "types: '-X', '--XYZ')");
            }
            if (find(reserved.begin(), reserved.end(), c)
                    != reserved.end()) {
                throw BackendException(
                    "reserved command line name: " + c +
                    " given for argument '" + a->get_name());
            }
            if (find(handled.begin(), handled.end(),
                     c) != handled.end()) {
                throw BackendException("multiple command "
                                       "line arguments named " + c);
            }

            handled.push_back(c);
        }
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


    /** Generates the header section of the file
     *
     */
    void PyHeader::generate(const vector<symbol::Argument *> &args)
    {
        unindent() << "#! /usr/bin/env python\n";
        unindent() << "# -*- coding: utf-8 -*-\n\n";
        unindent() << "import argparse\n";
        unindent() << "import sys\n";
        unindent() << "from collections import namedtuple\n\n";

        unindent() << "def parse_bool(s):\n";
        unindent() << "    return True if s.lower() == \"y\" "
                   "else False\n\n";

        generate_records(args);
    }

    /** Generates the record declarations
     *
     */
    void PyHeader::generate_records(const vector<symbol::Argument *> &args)
    {
        for (symbol::Argument *a : args) {
            auto t = a->get_type();
            if (t->record())
                generate_record(t->record());
            else if (t->list() && t->list()->elem()->record())
                generate_record(t->list()->elem()->record());
        }
    }

    /** Generates a record declaration using collections.namedtuple
     *
     */
    void PyHeader::generate_record(const RecordType *r)
    {
        string name = PyUtils::record_name(r);
        unindent() << name << " = namedtuple(\""
                   << name << "\", [";

        auto it = r->begin();

        while (it != r->end()) {
            unindent() << "\"" << (*it).name << "\"";
            if (++it != r->end())
                unindent() << ", ";
        }

        unindent() << "])\n\n";

        unindent() << "def parse_" << name << "(s):\n";
        unindent() << "    rs = \"";
        PyRecordColonDelim rcd(unindent());
        r->accept(rcd);
        unindent() << "\"\n";
        unindent() << "    l = s.split(':')\n\n";
        unindent() << "    if len(l) != " << r->no_of_fields() << ":\n";
        unindent() << "        raise argparse.ArgumentTypeError("
                   "\"Expected a string of type \" + rs)\n";
        unindent() << "    try:\n";
        unindent() << "        return " << name << "(";
        PyRecordCastList rcl(unindent());
        r->accept(rcl);
        unindent() << ")\n";
        unindent() << "    except:\n";
        unindent() << "        raise argparse.ArgumentTypeError("
                   "\"Expected a string of type \" + rs)\n";
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
        PyUtils::constant_to_stream(unindent(), p->data());
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
            if (m.name() == "lalign") {
                p->expression()->accept(*this);
                unindent() << ".ljust(";
                p->arguments()->expression->accept(*this);
                unindent() << ")";
            } else if (m.name() == "length") {
                unindent() << "len(";
                p->expression()->accept(*this);
                unindent() << ")";
            } else if (m.name() == "lower") {
                p->expression()->accept(*this);
                unindent() << ".lower()";
            } else if (m.name() == "ralign") {
                p->expression()->accept(*this);
                unindent() << ".rjust(";
                p->arguments()->expression->accept(*this);
                unindent() << ")";
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
                    auto a = p->arguments();
                    auto key = a->expression;
                    auto asc = a->next->expression;

                    unindent() << "sorted(";
                    p->expression()->accept(*this);
                    unindent() << ", reverse=not ";
                    asc->accept(*this);
                    unindent() << ", key=lambda r: getattr(r, ";
                    key->accept(*this);
                    unindent() << "))";
                }
            } else if (m.name() == "join") {
                p->arguments()->expression->accept(*this);
                unindent() << ".join(";
                p->expression()->accept(*this);
                unindent() << ")";
            }
        } else if (t->record()) {

        } else {

        }
    }

    void PyBody::visit(ast::SymbolRef *p)
    {
        if (p->symbol()->argument())
            unindent() << "_args." << p->symbol()->get_name();
        else if (p->symbol()->variable())
            unindent() << p->symbol()->get_name();
    }

    void PyBody::visit(ast::FieldRef *p)
    {
        p->record()->accept(*this);
        unindent() << "." << p->field();
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

    void PyBody::visit(ast::ForEachEnum *p)
    {
        if (p->statements()) {
            indent() << "for ";
            unindent() << p->index()->get_name();
            unindent() << ", ";
            unindent() << p->value()->get_name();
            unindent() << " in enumerate(";
            p->expression()->accept(*this);
            unindent() << "):\n";
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

    void PyBody::visit(ast::VariableList *p)
    {
        p->statement->accept(*this);
        if (p->next)
            p->next->accept(*this);
    }

    void PyBody::visit(ast::VariableAssignment *p)
    {
        indent() << p->variable()->get_name() << " = ";
        p->expression()->accept(*this);
        unindent() << "\n";
    }

    void PyBody::visit(ast::VariableDeclaration *p)
    {
        /* Just generate the assignment */
        p->assignment()->accept(*this);
    }

    void PyBody::binary(const string &s, ast::BinaryExpression *e)
    {
        unindent() << "(";
        e->lhs()->accept(*this);
        unindent() << " " << s << " ";
        e->rhs()->accept(*this);
        unindent() << ")";
    }

    /** Generates main() and the main method call
     *
     */
    void PyMain::generate(const vector<symbol::Argument *> &args)
    {
        indent() << "def main(argv=None):\n";
        indent_inc();
        indent() << "if argv is None:\n";
        indent() << "   argv = sys.argv\n\n";
        indent() << "cmd = argv[0]\n\n";

        generate_opts(args);
        unindent() << "\n";

        indent() << "generate(args, args._file)\n\n";

        indent_dec();

        indent() << "if __name__ == \"__main__\":\n";
        indent() << "    main()\n";
    }

    /** Generates all argparse functionality
     *
     */
    void PyMain::generate_opts(const vector<symbol::Argument *> &args)
    {
        indent() << "parser = argparse.ArgumentParser(description="
                 "\"Generated by TeGeL.\")\n";

        /* Add "output file" argument */
        indent() << "parser.add_argument('-o', type=argparse.FileType('w'), "
                 "default=sys.stdout, help='output to file instead of stdout', "
                 "dest='_file')\n";

        for (auto a : args) {
            PyUtils::generate_opt(indent(), a);
        }

        indent() << "args = parser.parse_args()\n";
    }

    void PyBackend::generate(ostream &os,
                             const vector<symbol::Argument *> &args,
                             ast::Statements *body)
    {
        if (body) {
            /* Validate the command line names */
            check_cmd(args);

            PyHeader h(os);
            PyBody b(os);
            PyMain m(os);

            h.generate(args);
            os << "\n";
            b.generate(body);
            os << "\n";
            m.generate(args);
        }
    }

    void PyBackend::check_cmd(const vector<symbol::Argument *> &args)
    {
        PyUtils::check_cmd(args, { "-h", "-o", "--help" } );
    }
}
