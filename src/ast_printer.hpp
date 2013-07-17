#include "ast.hpp"

namespace ast_printer {

    using namespace ast;

    /** Abstract Syntax Tree Printer used for debugging
     *
     */
    class AST_Printer : public AST_Visitor
    {
        public:
            virtual void visit(Statements *p) {
                print_ws();
                cerr << "Statements\n";
                indent++;

                p->statement()->accept(*this);
                if (p->next())
                    p->next()->accept(*this);

                indent--;
            }

            virtual void visit(And *p) {
                binary("And", p);
            }

            virtual void visit(Or *p) {
                binary("Or", p);
            }

            virtual void visit(Not *p) {
                print_ws();
                cerr << "Not\n";
                indent++;
                p->expression()->accept(*this);
                indent--;
            }

            virtual void visit(BoolEquals *p) {
                binary("==", p);
            }

            virtual void visit(LessThan *p) {
                binary("<", p);
            }

            virtual void visit(LessThanOrEqual *p) {
                binary("<=", p);
            }

            virtual void visit(GreaterThan *p) {
                binary(">", p);
            }

            virtual void visit(GreaterThanOrEqual *p) {
                binary(">=", p);
            }

            virtual void visit(Equals *p) {
                binary("==", p);
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

            virtual void visit(StringLessThan *p) {
                binary("String<", p);
            }

            virtual void visit(StringLessThanOrEqual *p) {
                binary("String<=", p);
            }

            virtual void visit(StringGreaterThan *p) {
                binary("String>", p);
            }

            virtual void visit(StringGreaterThanOrEqual *p) {
                binary("String>=", p);
            }

            virtual void visit(StringEquals *p) {
                binary("String==", p);
            }

            virtual void visit(StringRepeat *p) {
                binary("String*", p);
            }

            virtual void visit(StringConcat *p) {
                binary("String+", p);
            }

            virtual void visit(ListConcat *p) {
                binary("List+", p);
            }

            virtual void visit(Constant *p) {
                print_ws();
                cerr << "Constant(";
                p->data()->print(cerr);
                cerr << ")\n";
            }

            virtual void visit(MethodCall *p) {
                print_ws();
                cerr << "MethodCall(" << p->method().name() << ")\n";
                indent++;
                p->expression()->accept(*this);
                for (auto e = p->arguments(); e != nullptr;
                        e = e->next)
                    e->expression->accept(*this);
                indent--;
            }

            virtual void visit(SymbolRef *p) {
                print_ws();
                cerr << "SymbolRef(" << p->symbol()->get_name()
                     << ", " << p->symbol()->get_type()->str()
                     << ", " << p->symbol() << ")\n";
            }

            virtual void visit(FieldRef *p) {
                print_ws();
                cerr << "FieldRef";
                indent++;
                p->record()->accept(*this);
                print_ws();
                cerr << "." << p->field() << "\n";
            }

            virtual void visit(List *p) {
                print_ws();
                cerr << "List " << p->type()->str() << "\n";
                indent++;
                for (auto e = p->elements(); e != nullptr;
                        e = e->next) {
                    e->expression->accept(*this);
                }
                indent--;
            }

            virtual void visit(Conditional *p) {
                print_ws();
                cerr << "Conditional\n";
                indent++;
                p->if_node()->accept(*this);
                if (p->elif_nodes())
                    p->elif_nodes()->accept(*this);
                if (p->else_node())
                    p->else_node()->accept(*this);
                indent--;
            }

            virtual void visit(ForEach *p) {
                print_ws();
                cerr << "ForEach\n";
                indent++;
                print_ws();
                p->variable()->print(cerr);
                cerr <<"\n";
                p->expression()->accept(*this);
                if (p->statements())
                    p->statements()->accept(*this);
                indent--;
            }

            virtual void visit(ForEachEnum *p) {
                print_ws();
                cerr << "ForEachEnum\n";
                indent++;
                print_ws();
                p->index()->print(cerr);
                cerr <<"\n";
                print_ws();
                p->value()->print(cerr);
                cerr <<"\n";
                p->expression()->accept(*this);
                if (p->statements())
                    p->statements()->accept(*this);
                indent--;
            }

            virtual void visit(If *p) {
                print_ws();
                cerr << "If(" << p->table() << ")\n";
                indent++;
                p->condition()->accept(*this);
                if (p->statements())
                    p->statements()->accept(*this);
                indent--;
            }

            virtual void visit(Elif *p) {
                print_ws();
                cerr << "Elif(" << p->table() << ")\n";
                indent++;
                p->condition()->accept(*this);
                if (p->statements())
                    p->statements()->accept(*this);
                if (p->next())
                    p->next()->accept(*this);
                indent--;
            }

            virtual void visit(Else *p)  {
                print_ws();
                cerr << "Else(" << p->table() << ")\n";
                indent++;
                if (p->statements())
                    p->statements()->accept(*this);
                else {
                    print_ws();
                    cerr << "No statement";
                }
                indent--;
            }

            virtual void visit(Text *p) {
                print_ws();
                cerr << "Text('" << p->text() << "')\n";
            }

            virtual void visit(InlinedExpression *p) {
                print_ws();
                cerr << "InlinedExpression\n";
                indent++;
                p->expression()->accept(*this);
                indent--;
            }

            virtual void visit(VariableAssignment *p) {
                print_ws();
                cerr << "VariableAssignment\n";
                indent++;
                print_ws();
                p->variable()->print(cerr);
                cerr <<"\n";
                p->expression()->accept(*this);
                indent--;
            }

            virtual void visit(VariableDeclaration *p) {
                print_ws();
                cerr << "VariableDeclaration\n";
                indent++;
                print_ws();
                p->variable()->print(cerr);
                cerr <<"\n";
                p->assignment()->accept(*this);
                indent--;
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
