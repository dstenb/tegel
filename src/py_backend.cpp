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
                : os_(os), index_(0) {}

            virtual void visit(const RecordType *p) {
                auto it = p->begin();
                while (it != p->end()) {
                    (*it).type->accept(*this);
                    index_++;
                    if (++it != p->end())
                        os_ << ", ";
                }
            }

            virtual void visit(const BoolType *) {
                os_ << "parse_bool(l[" << index_ << "])";
            }

            virtual void visit(const IntType *) {
                os_ << "int(l[" << index_ << "])";
            }

            virtual void visit(const StringType *) {
                os_ << "l[" << index_ << "]";
            }

            virtual void visit(const ListType *) { }
        private:
            ostream &os_;
            int index_;
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


    /** Generates the header section of the file
     *
     */
    void PyHeader::generate()
    {
        unindent() << "#! /usr/bin/env python\n";
        unindent() << "# -*- coding: utf-8 -*-\n\n";
        unindent() << "from __future__ import unicode_literals\n\n";
        unindent() << "import argparse\n";
        unindent() << "import errno\n";
        unindent() << "import os\n";
        unindent() << "import sys\n";
        unindent() << "import textwrap\n";
        unindent() << "from collections import namedtuple\n\n";

        unindent() << "def parse_bool(s):\n";
        unindent() << "    return True if s.lower() == \"y\" "
                   "else False\n\n";

        unindent() << "def mkdir_chdir(path):\n";
        unindent() << "    try:\n";
        unindent() << "        os.mkdir(path)\n";
        unindent() << "    except OSError as e:\n";
        unindent() << "        if e.errno != errno.EEXIST:\n";
        unindent() << "            print('Can\\'t create %s: %s' % "
                   "(path, e.strerror))\n";
        unindent() << "            sys.exit(1)\n";
        unindent() << "    try:\n";
        unindent() << "        os.chdir(path)\n";
        unindent() << "    except OSError as e:\n";
        unindent() << "        print('%s: %s' % (e.strerror, path))\n\n";

        unindent() << "def _input(s = ''):\n";
        unindent() << "    if sys.version_info >= (3, 0):\n";
        unindent() << "        return input(s)\n";
        unindent() << "    else:\n";
        unindent() << "        return raw_input(s)\n\n";

        unindent() << "def open_file(path, ask):\n";
        unindent() << "    try:\n";
        unindent() << "        fd = os.open(path, "
                   "os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o644)\n";
        unindent() << "        return os.fdopen(fd, 'w')\n";
        unindent() << "    except OSError as e:\n";
        unindent() << "        if e.errno == errno.EEXIST:\n";
        unindent() << "            while ask:\n";
        unindent() << "                c = _input('%s already exists, "
                   "overwrite? [Y/n]: ' % path)\n";
        unindent() << "                if (c in [ 'y', 'Y', '' ]):\n";
        unindent() << "                    ask = False\n";
        unindent() << "                elif (c in [ 'n', 'N' ]):\n";
        unindent() << "                    return None\n";
        unindent() << "            return open(path, 'w')\n";
        unindent() << "        else:\n";
        unindent() << "            raise\n\n";

        unindent() << "def write(f, s):\n";
        unindent() << "    if sys.version < '3':\n";
        unindent() << "        f.write(s.encode('utf-8'))\n";
        unindent() << "    else:\n";
        unindent() << "        f.buffer.write(s.encode('utf-8'))\n\n";

        unindent() << "def to_str(x):\n";
        unindent() << "    if type(x) == bool:\n";
        unindent() << "        return 'true' if x else 'false'\n";
        unindent() << "    else:\n";
        unindent() << "        return str(x)\n\n";

        /* The loop record variable in loops are instances of Loop instead of
         * tuple_list to improve performance (by not having to create a new
         * tuple every iteration); however, when other variables are
         * assigned to their values, e.g. %with loop a = loop, a tuple is
         * created (see copy())
         */
        unindent() << "class Loop:\n";
        unindent() << "    def __init__(self, list):\n";
        unindent() << "        self.list = list\n";
        unindent() << "        self.length = len(list)\n";
        unindent() << "        self.index = 0\n";
        unindent() << "        self.first = True\n";
        unindent() << "        self.last = (self.length == 1)\n\n";
        unindent() << "    def tuple(self):\n";
        unindent() << "        return tuple_loop(self.index, self.first, "
                   "self.last, self.length)\n\n";
        unindent() << "    def update(self):\n";
        unindent() << "        self.index = self.index + 1\n";
        unindent() << "        self.first = False\n";
        unindent() << "        self.last = (self.index == self.length - 1)"
                   "\n\n\n";

        unindent() << "def copy(o):\n";
        unindent() << "    if (isinstance(o, Loop)):\n";
        unindent() << "        return o.tuple()\n";
        unindent() << "    return o\n\n";

        generate_records();
    }

    /** Generates the record declarations
     *
     */
    void PyHeader::generate_records()
    {
        for (const RecordType *r : TypeFactory::get_records())
            generate_record(r);
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
        windent("def generate(_args, _file):\n");
        indent_inc();
        if (body)
            body->accept(*this);
        else
            indent() << "pass\n";
        indent_dec();
    }

    void PyBody::generate(ParseData *tgp, const map<string, ParseData *> &tgl)
    {
        tgl_ = tgl;

        windent("def generate(_args, _file, _body=\"\"):\n");
        indent_inc();
        windent("if _body == \"\":\n");
        indent_inc();
        if (tgp->body)
            tgp->body->accept(*this);
        else
            indent() << "pass\n";
        indent_dec();
        for (auto it = tgl.begin(); it != tgl.end(); ++it) {
            indent() << "elif _body == \"" << it->first << "\":\n";
            indent_inc();
            if (it->second->body)
                it->second->body->accept(*this);
            else
                indent() << "pass\n";
            indent_dec();
        }
    }

    void PyBody::visit(ast::Statements *p)
    {
        p->statement()->accept(*this);
        if (p->next())
            p->next()->accept(*this);
    }

    void PyBody::visit(ast::TernaryIf *p)
    {
        write("(%a if %a else %a)", p->if_true(),
              p->condition(), p->if_false());
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
        write("not %a", p->expression());
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
        auto t = p->expression()->type();
        auto m = p->method();
        auto name = m.name();

        ast::Expression *e = p->expression();
        ast::Expression *a0 = nullptr;
        ast::Expression *a1 = nullptr;

        if (p->arguments()) {
            a0 = p->arguments()->expression;
            if (p->arguments()->next)
                a1 = p->arguments()->next->expression;
        }

        if (t == type::TypeFactory::get("bool")) {
            if (name == "str") {
                write("to_str(%a)", e);
            }
        } else if (t == type::TypeFactory::get("int")) {
            if (name == "downto") {
                write("list(reversed(range(%a, %a + 1)))", a0, e);
            } else if (name == "str") {
                write("str(%a)", e);
            } else if (name == "upto") {
                write("list(range(%a, %a + 1))", e, a0);
            }
        } else if (t == type::TypeFactory::get("string")) {
            if (name == "lalign") {
                write("%a.ljust(%a)", e, a0);
            } else if (name == "length") {
                write("len(%a)", e);
            } else if (name == "lower") {
                write("%a.lower()", e);
            } else if (name == "ralign") {
                write("%a.rjust(%a)", e, a0);
            } else if (name == "title") {
                write("%a.title()", e);
            } else if (name == "upper") {
                write("%a.upper()", e);
            } else if (name == "replace") {
                write("%a.replace(%a, %a)", e, a0, a1);
            } else if (name == "wrap") {
                write("textwrap.wrap(%a, %a)", e, a0);
            }
        } else if (t->record()) {
            if (name == "elems") {
                write("map(lambda x: to_str(x), list(%a))", e);
            }
        } else if (t->list()) {
            if (name == "size") {
                write("len(%a)", e);
            } else if (name == "sort") {
                if (t->list()->elem()->primitive()) {
                    write("sorted(%a, reverse=not %a)", e, a0);
                } else if (t->list()->elem()->record()) {
                    write("sorted(%a, reverse=not %a, "
                          "key=lambda r: getattr(r, %a))", e, a1, a0);
                }
            } else if (name == "join") {
                write("%a.join(%a)", a0, e);
            }
        }
    }

    void PyBody::visit(ast::LambdaExpression *p)
    {
        write("lambda ");
        for (auto v = p->variables; v != nullptr; v = v->next) {
            write(table_.get(v->statement->variable()).c_str());
            if (v->next)
                write(", ");
        }

        write(": %a", p->expression);
    }

    void PyBody::visit(ast::FunctionCall *p)
    {
        if (p->name == "filter") {
            write("filter(%a, %a)", p->args->get_lambda(0),
                  p->args->get_expression(1));
        } else if (p->name == "map") {
            write("map(%a, %a)", p->args->get_lambda(0),
                  p->args->get_expression(1));
        }
    }

    void PyBody::visit(ast::SymbolRef *p)
    {
        if (p->symbol()->argument())
            write("_args[\"%s\"]", p->symbol()->get_name().c_str());
        else if (p->symbol()->variable())
            write("%s", table_.get(p->symbol()).c_str());
    }

    void PyBody::visit(ast::FieldRef *p)
    {
        write("%a.%s", p->record(), p->field().c_str());
    }

    void PyBody::visit(ast::List *p)
    {
        write("[");
        for (auto e = p->elements(); e != nullptr; e = e->next) {
            e->expression->accept(*this);
            if (e->next)
                unindent() << ", ";
        }
        write("]");
    }

    void PyBody::visit(ast::Record *p)
    {
        write("%s(", PyUtils::record_name(p->type()).c_str());
        for (auto e = p->fields(); e != nullptr; e = e->next) {
            e->expression->accept(*this);
            if (e->next)
                write(", ");
        }
        write(")");
    }

    void PyBody::visit(ast::FuncArgList *) {

    }

    void PyBody::visit(ast::FuncArgExpression *) {

    }

    void PyBody::visit(ast::FuncArgLambda *) {

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
            windent("%s = Loop(%a)\n", table_.get(p->loop_variable()).c_str(),
                    p->expression());
            windent("for %s in %s.list:\n", table_.get(p->variable()).c_str(),
                    table_.get(p->loop_variable()).c_str());
            indent_inc();
            p->statements()->accept(*this);
            indent() << table_.get(p->loop_variable()) << ".update()\n";
            indent_dec();
        }
    }

    void PyBody::visit(ast::ForEachEnum *p)
    {
        if (p->statements()) {
            windent("for %s, %s in enumerate(%a):\n",
                    table_.get(p->index()).c_str(),
                    table_.get(p->value()).c_str(),
                    p->expression());
            indent_inc();
            p->statements()->accept(*this);
            indent_dec();
        }
    }

    void PyBody::visit(ast::If *p)
    {
        windent("if %a:\n", p->condition());
        indent_inc();
        if (p->statements())
            p->statements()->accept(*this);
        indent_dec();
    }

    void PyBody::visit(ast::Elif *p)
    {
        windent("elif %a:\n", p->condition());
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
            windent("else:\n");
            indent_inc();
            p->statements()->accept(*this);
            indent_dec();
        }
    }

    void PyBody::visit(ast::Text *p)
    {
        windent("write(_file, \"%s\")\n", Escaper()(p->text()).c_str());
    }

    void PyBody::visit(ast::InlinedExpression *p)
    {
        windent("write(_file, %a)\n", p->expression());
    }

    void PyBody::visit(ast::VariableList *p)
    {
        p->statement->accept(*this);
        if (p->next)
            p->next->accept(*this);
    }

    void PyBody::visit(ast::VariableAssignment *p)
    {
        windent("%s = copy(%a)\n", table_.get(p->variable()).c_str(),
                p->expression());
    }

    void PyBody::visit(ast::VariableDeclaration *p)
    {
        /* Just generate the assignment */
        p->assignment()->accept(*this);
    }

    void PyBody::visit(ast::Create *p)
    {
        windent("try:\n");
        windent("    f = open_file(%a, %s)\n", p->out,
                (p->ow_ask ? "True" : "False"));
        windent("    __args = {");
        auto pd = tgl_[p->tgl];
        for (symbol::Argument *a : pd->arguments) {
            write("\"%s\": ", a->get_name().c_str());
            auto it = p->args.find(a->get_name());
            if (it != p->args.end()) {
                it->second->accept(*this);
            } else {
                PyConstToStream c(unindent());
                a->get("default")->get()->accept(c);
            }
            write(", ");
        }
        write("}\n");
        windent("    if f:\n");
        windent("        try:\n");
        windent("            generate(__args, f, \"%s\")\n", p->tgl.c_str());
        windent("        finally:\n");
        windent("            f.close()\n");
        windent("except IOError as e:\n");
        windent("    print('Can\\'t create %%s: %%s' % (%a, e.strerror()))\n",
                p->out);
        windent("    pass\n");
    }

    void PyBody::binary(const string &s, ast::BinaryExpression *e)
    {
        write("(%a %s %a)", e->lhs(), s.c_str(), e->rhs());
    }

    /** Generates main() and the main method call
     *
     */
    void PyMain::generate(const vector<symbol::Argument *> &args,
                          const vector<PyExtraArgument> &extra, bool mkdir)
    {
        indent() << "def main(argv=None):\n";
        indent_inc();
        indent() << "if argv is None:\n";
        indent() << "   argv = sys.argv\n\n";

        generate_opts(args, extra);
        unindent() << "\n";

        if (mkdir)
            indent() << "mkdir_chdir(args._dir)\n\n";

        indent() << "if sys.version < '3':\n";
        indent() << "    def convert_elem(v):\n";
        indent() << "        if type(v) == str:\n";
        indent() << "            return v.decode(sys.stdin.encoding)\n";
        indent() << "        elif type(v) in [ bool, int ]:\n";
        indent() << "            return v\n";
        indent() << "        else:\n";
        indent() << "            l = [ (v[i].decode('utf-8') if (type(v[i]) == str)\n";
        indent() << "                else v[i]) for i in range(len(v)) ]\n";
        indent() << "            return v.__new__(type(v), *l)\n";
        indent() << "\n";
        indent() << "    for a in vars(args):\n";
        indent() << "        v = getattr(args, a)\n";
        indent() << "        if not a.startswith('_'):\n";
        indent() << "            if type(v) == list:\n";
        indent() <<
                 "                setattr(args, a, [ convert_elem(e) for e in v ])\n";
        indent() << "            else:\n";
        indent() << "                setattr(args, a, convert_elem(v))\n";

        indent() << "generate(vars(args), args._file)\n\n";

        indent_dec();

        indent() << "if __name__ == \"__main__\":\n";
        indent() << "    main()\n";
    }

    /** Generates all argparse functionality
     *
     */
    void PyMain::generate_opts(const vector<symbol::Argument *> &args,
                               const vector<PyExtraArgument> &extra)
    {
        indent() << "parser = argparse.ArgumentParser(description="
                 "\"Generated by TeGeL.\")\n";

        for (auto e : extra) {
            indent() << "parser.add_argument(" << e.name << ", type=" <<
                     e.type << ", default=" << e.default_value << ", help=" <<
                     e.help << ", dest=" << e.destination << ")\n";
        }

        for (auto a : args) {
            PyUtils::generate_opt(indent(), a);
        }

        indent() << "args = parser.parse_args()\n";
    }

    void PyBackend::generate(ostream &os,
                             const vector<symbol::Argument *> &args,
                             ast::Statements *body)
    {
        vector<PyExtraArgument> extra = {
            {   "'-o'", "argparse.FileType('w')", "sys.stdout",
                "'output to file instead of stdout'", "'_file'"
            },
        };

        /* Validate the command line names */
        check_cmd(args);

        PyHeader h(os);
        PyBody b(os);
        PyMain m(os);

        h.generate();
        os << "\n";
        b.generate(body);
        os << "\n";
        m.generate(args, extra, false);
    }

    void PyBackend::check_cmd(const vector<symbol::Argument *> &args)
    {
        PyUtils::check_cmd(args, { "-h", "-o", "--help" } );
    }

    void PyTgpBackend::generate(ostream &os, ParseData *tgp_data,
                                map<string, ParseData *> &tgl_data)
    {
        vector<PyExtraArgument> extra = {
            {   "'-o'", "argparse.FileType('w')", "sys.stdout",
                "'output to file instead of stdout'", "'_file'"
            },
            {   "'--dir'", "str", "'.'", "'select output directory (will "
                "create the directory if non-existant)'", "'_dir'"
            }
        };

        /* Validate the command line names */
        check_cmd(tgp_data->arguments);

        PyHeader h(os);
        PyBody b(os);
        PyMain m(os);

        h.generate();
        os << "\n";
        b.generate(tgp_data, tgl_data);
        os << "\n";
        m.generate(tgp_data->arguments, extra, true);
    }

    void PyTgpBackend::check_cmd(const vector<symbol::Argument *> &args)
    {
        PyUtils::check_cmd(args, { "-h", "-o", "--help", "--dir" } );
    }
}
