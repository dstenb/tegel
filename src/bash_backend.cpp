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

    void BashBody::visit(ast::TernaryIf *p)
    {
    }

    void BashBody::visit(ast::And *p)
    {
    }

    void BashBody::visit(ast::Or *p)
    {
    }

    void BashBody::visit(ast::Not *p)
    {
    }

    void BashBody::visit(ast::BoolEquals *p)
    {
    }

    void BashBody::visit(ast::LessThan *p)
    {
    }

    void BashBody::visit(ast::LessThanOrEqual *p)
    {
    }

    void BashBody::visit(ast::GreaterThan *p)
    {
    }

    void BashBody::visit(ast::GreaterThanOrEqual *p)
    {
    }

    void BashBody::visit(ast::Equals *p)
    {
    }

    void BashBody::visit(ast::Plus *p)
    {
    }

    void BashBody::visit(ast::Minus *p)
    {
    }

    void BashBody::visit(ast::Times *p)
    {
    }

    void BashBody::visit(ast::StringLessThan *p)
    {
    }

    void BashBody::visit(ast::StringLessThanOrEqual *p)
    {
    }

    void BashBody::visit(ast::StringGreaterThan *p)
    {
    }

    void BashBody::visit(ast::StringGreaterThanOrEqual *p)
    {
    }

    void BashBody::visit(ast::StringEquals *p)
    {
        p->lhs()->accept(*this);
        unindent() << " == ";
        p->rhs()->accept(*this);
    }

    void BashBody::visit(ast::StringRepeat *p)
    {
    }

    void BashBody::visit(ast::StringConcat *p)
    {
    }

    void BashBody::visit(ast::ListConcat *p)
    {
    }

    void BashBody::visit(ast::Constant *p)
    {
    }

    void BashBody::visit(ast::MethodCall *p)
    {

    }

    void BashBody::visit(ast::SymbolRef *p)
    {
        if (p->symbol()->argument())
            unindent() << "args[\"" << p->symbol()->get_name() << "\"";
        else if (p->symbol()->variable())
            unindent() << p->symbol()->get_name();
    }

    void BashBody::visit(ast::FieldRef *p)
    {
    }

    void BashBody::visit(ast::List *p)
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

    void BashBody::visit(ast::ForEach *p)
    {
    }

    void BashBody::visit(ast::ForEachEnum *p)
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

    void BashBody::visit(ast::InlinedExpression *p)
    {
    }

    void BashBody::visit(ast::VariableList *p)
    {
    }

    void BashBody::visit(ast::VariableAssignment *p)
    {
    }

    void BashBody::visit(ast::VariableDeclaration *p)
    {
    }

    void BashBody::binary(const string &s, ast::BinaryExpression *e)
    {
    }


    void BashBackend::generate(ostream &os,
                               const vector<symbol::Argument *> &args,
                               ast::Statements *body)
    {
        if (body) {
            BashBody b(os);
            b.generate(body);
        }
    }
}