#ifndef __PYTHON_BACKEND_H__
#define __PYTHON_BACKEND_H__

#include <sstream>
#include <string>

using namespace std;

#include "backend.hpp"
#include "common.hpp"
#include "data.hpp"
#include "type.hpp"
#include "py_helpers.hpp"

using namespace py_helpers;

namespace py_backend {

    struct PyExtraArgument
    {
        string name;
        string type;
        string default_value;
        string help;
        string destination;
    };

    struct PyUtils
    {
        static ostream &constant_to_stream(ostream &, const ConstantData *);
        static string record_name(const RecordType *);
        static bool is_short_cmd(const string &);
        static bool is_long_cmd(const string &);
        static bool valid_cmd_format(const string &);
        static void generate_opt(ostream &, symbol::Argument *);
        static void check_cmd(const vector<symbol::Argument *> &,
                              const vector<string> &);
    };

    class PyVariableCreator : public AsciiStringCreator
    {
        public:
            PyVariableCreator()
                : AsciiStringCreator("") {}
    };

    class PySymbolTable : public BackendUntypedSymbolTable<PyVariableCreator>
    {
        public:
            PySymbolTable()
                : BackendUntypedSymbolTable() {}
    };

    class PyHeader : public PyWriter
    {
        public:
            PyHeader(ostream &os)
                : PyWriter(os, 0) {}

            void generate(void);
        private:
            /** Generate the named tuples for all the defined records
             *
             */
            void generate_records(void);
            void generate_record(const RecordType *);
    };

    class PyBody : public PyWriter, public ast::AST_Visitor
    {
        public:
            PyBody(ostream &os)
                : PyWriter(os, 0), tgl_(), table_() {}

            /** Generates a body generation function named "generate"
             *
             *
             */
            void generate(ast::Statements *body);
            void generate(ParseData *, const map<string, ParseData *> &);

            virtual void visit(ast::TernaryIf *);
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
            virtual void visit(ast::FieldRef *);
            virtual void visit(ast::List *);
            virtual void visit(ast::Record *);
            virtual void visit(ast::Statements *);
            virtual void visit(ast::Conditional *);
            virtual void visit(ast::ForEach *);
            virtual void visit(ast::ForEachEnum *);
            virtual void visit(ast::If *);
            virtual void visit(ast::Elif *);
            virtual void visit(ast::Else *);
            virtual void visit(ast::Text *);
            virtual void visit(ast::InlinedExpression *);
            virtual void visit(ast::VariableList *);
            virtual void visit(ast::VariableAssignment *);
            virtual void visit(ast::VariableDeclaration *);
            virtual void visit(ast::Create*);
        private:
            void binary(const string &s, ast::BinaryExpression *e);

            map<string, ParseData *> tgl_;
            PySymbolTable table_;
    };

    class PyMain : public PyWriter
    {
        public:
            PyMain(ostream &os)
                : PyWriter(os, 0) {}

            void generate(const vector<symbol::Argument *> &,
                          const vector<PyExtraArgument> &, bool);
        private:
            void generate_opts(const vector<symbol::Argument *> &,
                               const vector<PyExtraArgument> &);
    };

    /** Backend for .tgl files
     *
     */
    class PyBackend : public Backend
    {
        public:
            void generate(ostream &, const vector<symbol::Argument *> &,
                          ast::Statements *);
        private:
            void check_cmd(const vector<symbol::Argument *> &args);
    };

    /** Backend for .tgp files
     *
     */
    class PyTgpBackend : public TgpBackend
    {
        public:
            void generate(ostream &, ParseData *, map<string, ParseData *> &);
        private:
            void check_cmd(const vector<symbol::Argument *> &args);
    };
}

#endif
