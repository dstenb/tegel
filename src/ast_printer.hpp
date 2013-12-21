#include "ast.hpp"
#include "common.hpp"

namespace ast_printer {

    using namespace ast;

    /** Abstract Syntax Tree Printer used for debugging
     *
     */
    class AST_Printer : public AST_Visitor
    {
        public:
            AST_Printer()
                : indent(0) {}

            virtual void visit(Statements *p) {
                p->statement()->accept(*this);
                if (p->next())
                    p->next()->accept(*this);
            }

            virtual void visit(TernaryIf *p) {
                print_ws();
                cerr << "?\n";
                indent++;
                p->condition()->accept(*this);
                indent++;
                p->if_true()->accept(*this);
                print_ws();
                cerr << ":\n";
                p->if_false()->accept(*this);
                indent -= 2;
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
                cerr << "FieldRef\n";
                indent++;
                p->record()->accept(*this);
                print_ws();
                cerr << "." << p->field() << "\n";
                indent--;
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

            virtual void visit(Record *p) {
                print_ws();
                cerr << "Record " << p->type()->str() << "\n";
                indent++;
                for (auto e = p->fields(); e != nullptr;
                        e = e->next) {
                    e->expression->accept(*this);
                }
                indent--;
            }

            virtual void visit(LambdaExpression *p) {
                print_ws();
                cerr << "LambdaExpression(" << p->table << ")\n";
                indent++;
                p->variables->accept(*this);
                assert(p->expression);
                p->expression->accept(*this);
                indent--;
            }

            virtual void visit(FunctionCall *p){
                print_ws();
                cerr << "FunctionCall(" << p->type()->str()
                    << ", " << p->name << ")\n";
                indent++;
                p->args->accept(*this);
                indent--;
            }

            virtual void visit(FuncArgList *p) {
                print_ws();
                cerr << "FuncArgList\n";
                indent++;
                p->arg->accept(*this);
                if (p->next)
                    p->next->accept(*this);
                indent--;
            }

            virtual void visit(FuncArgExpression *p) {
                print_ws();
                cerr << "FuncArgExpression\n";
                indent++;
                p->value->accept(*this);
                indent--;
                print_ws();
                cerr << ")\n";
            }

            virtual void visit(FuncArgLambda *p) {
                print_ws();
                cerr << "FuncArgLambda\n";
                indent++;
                p->value->accept(*this);
                indent--;
                print_ws();
                cerr << ")\n";
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
                cerr << "Text('" << Escaper()(p->text()) << "')\n";
            }

            virtual void visit(InlinedExpression *p) {
                print_ws();
                cerr << "InlinedExpression\n";
                indent++;
                p->expression()->accept(*this);
                indent--;
            }

            virtual void visit(VariableList *p) {
                print_ws();
                cerr << "VariableList\n";
                indent++;
                p->statement->accept(*this);
                if (p->next)
                    p->next->accept(*this);
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
                if (p->assignment())
                    p->assignment()->accept(*this);
                indent--;
            }

            virtual void visit(Create *p) {
                print_ws();
                cerr << "Create(tgl_file=" << p->tgl << ", ow_ask="
                     << (p->ow_ask ? "true" : "false") << ")\n";
                indent++;
                p->out->accept(*this);
                /* TODO */
                for (auto a = p->args.begin(); a != p->args.end(); a++) {
                    print_ws();
                    cerr << a->first << "\n";
                    indent++;
                    a->second->accept(*this);
                    indent--;
                }
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

            void print_ws() {
                for (int i = 0; i < indent; i++)
                    cerr << " ";
            }

            int indent;
    };
}
