#include "bash_backend.hpp"

namespace bash_backend
{

    ostream &BashWriter::indent()
    {
        for (unsigned i = 0; i < indentation_; i++)
            os_ << '\t';
        return os_;
    }

    ostream &BashWriter::unindent()
    {
        return os_;
    }

    void BashWriter::indent_dec()
    {
        if (indentation_ > 0)
            indentation_--;
    }

    void BashWriter::indent_inc()
    {
        indentation_++;
    }


    void BashBody::generate(ast::Statements *body)
    {
        indent() << "function generate {\n";
        indent_inc();
        body->accept(*this);
        indent_dec();
        indent() << "}\n";
    }

    void BashBody::visit(ast::Statements *p)
    {
        p->statement()->accept(*this);
        if (p->next())
            p->next()->accept(*this);
    }

    void BashBody::visit(ast::TernaryIf *)
    {
    }

    void BashBody::visit(ast::And *)
    {
    }

    void BashBody::visit(ast::Or *)
    {
    }

    void BashBody::visit(ast::Not *)
    {
    }

    void BashBody::visit(ast::BoolEquals *)
    {
    }

    void BashBody::visit(ast::LessThan *)
    {
    }

    void BashBody::visit(ast::LessThanOrEqual *)
    {
    }

    void BashBody::visit(ast::GreaterThan *)
    {
    }

    void BashBody::visit(ast::GreaterThanOrEqual *)
    {
    }

    void BashBody::visit(ast::Equals *)
    {
    }

    void BashBody::visit(ast::Plus *)
    {
    }

    void BashBody::visit(ast::Minus *)
    {
    }

    void BashBody::visit(ast::Times *)
    {
    }

    void BashBody::visit(ast::StringLessThan *)
    {
    }

    void BashBody::visit(ast::StringLessThanOrEqual *)
    {
    }

    void BashBody::visit(ast::StringGreaterThan *)
    {
    }

    void BashBody::visit(ast::StringGreaterThanOrEqual *)
    {
    }

    void BashBody::visit(ast::StringEquals *p)
    {
        p->lhs()->accept(*this);
        unindent() << " == ";
        p->rhs()->accept(*this);
    }

    void BashBody::visit(ast::StringRepeat *)
    {
    }

    void BashBody::visit(ast::StringConcat *)
    {
    }

    void BashBody::visit(ast::ListConcat *)
    {
    }

    void BashBody::visit(ast::Constant *)
    {
    }

    void BashBody::visit(ast::MethodCall *)
    {

    }

    void BashBody::visit(ast::SymbolRef *p)
    {
        if (p->symbol()->argument())
            unindent() << "args[\"" << p->symbol()->get_name() << "\"";
        else if (p->symbol()->variable())
            unindent() << p->symbol()->get_name();
    }

    void BashBody::visit(ast::FieldRef *)
    {
    }

    void BashBody::visit(ast::List *)
    {
    }

    void BashBody::visit(ast::Record *)
    {
    }

    void BashBody::visit(ast::Conditional *p)
    {
        p->if_node()->accept(*this);
        if (p->elif_nodes())
            p->elif_nodes()->accept(*this);
        if (p->else_node())
            p->else_node()->accept(*this);
        indent() << "fi\n";
    }

    void BashBody::visit(ast::ForEach *)
    {
    }

    void BashBody::visit(ast::ForEachEnum *)
    {
    }

    void BashBody::visit(ast::If *p)
    {
        indent() << "if [ ";
        p->condition()->accept(*this);

        unindent() << "] ; then\n";
        indent_inc();
        if (p->statements())
            p->statements()->accept(*this);
        indent_dec();
    }

    void BashBody::visit(ast::Elif *p)
    {
        indent() << "elif [";
        p->condition()->accept(*this);
        unindent() << "] ; then\n";
        indent_inc();
        if (p->statements())
            p->statements()->accept(*this);
        indent_dec();
        if (p->next())
            p->next()->accept(*this);
    }

    void BashBody::visit(ast::Else *p)
    {
        if (p->statements()) {
            indent() << "else\n";
            indent_inc();
            p->statements()->accept(*this);
            indent_dec();
        }
    }

    void BashBody::visit(ast::Text *p)
    {
        indent() << "echo \"" << Escaper()(p->text()) << "\"\n";
    }

    void BashBody::visit(ast::InlinedExpression *)
    {
    }

    void BashBody::visit(ast::VariableList *)
    {
    }

    void BashBody::visit(ast::VariableAssignment *)
    {
    }

    void BashBody::visit(ast::VariableDeclaration *)
    {
    }

    void BashBody::visit(ast::Create *)
    {
    }

    void BashBody::binary(const string &, ast::BinaryExpression *)
    {
    }


    void BashBackend::generate(ostream &os,
                               const vector<symbol::Argument *> &,
                               ast::Statements *body)
    {
        if (body) {
            BashBody b(os);
            b.generate(body);
        }
    }
}
