#ifndef __BASH_BACKEND_H__
#define __BASH_BACKEND_H__

#include <sstream>
#include <string>

using namespace std;

#include "backend.hpp"
#include "common.hpp"
#include "type.hpp"

namespace bash_backend {

    /** BashWriter class
     *
     * BashWriter provides python indentation (i.e. levels of 4 spaces) to an ostream
     *
     */
    class BashWriter
    {
        public:
            BashWriter(ostream &os, unsigned indentation = 0)
                : os_(os), indentation_(indentation) {}

            virtual ~BashWriter() {}
        protected:
            ostream &indent();
            ostream &unindent();

            void indent_dec();
            void indent_inc();
        private:
            ostream &os_;
            unsigned indentation_;
    };

    class BashVariableCreator : public AsciiStringCreator
    {
        public:
            BashVariableCreator()
                : AsciiStringCreator("") {}
    };

    class BashSymbolTable : public BackendUntypedSymbolTable<BashVariableCreator>
    {
        public:
            BashSymbolTable()
                : BackendUntypedSymbolTable() {}
    };

    class BashBody : public BashWriter, public ast::AST_Visitor
    {
        public:
            BashBody(ostream &os)
                : BashWriter(os), table_() {}

            void generate(ast::Statements *body);

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

            BashSymbolTable table_;
    };

    class BashMain : public BashWriter
    {
        public:
            BashMain(ostream &os)
                : BashWriter(os) {}

            void generate(const vector<symbol::Argument *> &);
        private:
            void generate_opts(const vector<symbol::Argument *> &);
    };

    class BashBackend : public Backend
    {
        public:
            void generate(ostream &, const vector<symbol::Argument *> &,
                          ast::Statements *);
        private:
    };

}

#endif
