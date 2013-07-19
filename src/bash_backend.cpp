#include "bash_backend.hpp"

namespace bash_backend
{

    void BashBody::generate(ast::Statements *body)
    {
        os_ << "function generate {\n";
        os_ << "}\n";
        body->accept(*this);
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
    }

    void BashBody::visit(ast::FieldRef *p)
    {
    }

    void BashBody::visit(ast::List *p)
    {
    }

    void BashBody::visit(ast::Conditional *p)
    {
    }

    void BashBody::visit(ast::ForEach *p)
    {
    }

    void BashBody::visit(ast::ForEachEnum *p)
    {
    }

    void BashBody::visit(ast::If *p)
    {
    }

    void BashBody::visit(ast::Elif *p)
    {
    }

    void BashBody::visit(ast::Else *p)
    {
    }

    void BashBody::visit(ast::Text *p)
    {
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
