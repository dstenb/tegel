#ifndef __AST_H__
#define __AST_H__

using namespace std;

#include "constant.hpp"
#include "symbol.hpp"
#include "type.hpp"

using namespace constant;
using namespace type;

namespace ast {

    class AST_Node;
    class AST_Visitor;

    /** Abstract AST node class
     *
     * An AST_Node object represents a node in the AST of the body section.
     *
     * The nodes are traversed using the visitor pattern (see AST_Visitor)
     */
    class AST_Node
    {
        public:
            virtual ~AST_Node() {}

            virtual void accept(AST_Visitor &) = 0;
    };

    class Statements;

    class Expression;
    class BinaryExpression;
    class UnaryExpression;
    class And;
    class Or;
    class Not;
    class BoolEquals;
    class LessThan;
    class LessThanOrEqual;
    class GreaterThan;
    class GreaterThanOrEqual;
    class Equals;
    class Plus;
    class Minus;
    class Times;
    class StringLessThan;
    class StringLessThanOrEqual;
    class StringGreaterThan;
    class StringGreaterThanOrEqual;
    class StringEquals;
    class StringRepeat;
    class StringConcat;
    class ListConcat;
    class Constant;
    class MethodCall;
    class SymbolRef;
    class FieldRef;
    class List;
    class ExpressionList;

    class Statement;
    class Conditional;
    class Scope;
    class ForEach;
    class ForEachEnum;
    class If;
    class Elif;
    class Else;
    class Text;
    class InlinedExpression;
    class VariableStatement;
    class VariableList;
    class VariableDeclaration;
    class VariableAssignment;

    /** Abstract expression base class
     *
     * An Expression object represents a part of an expression. It has a type
     * associated with it.
     */
    class Expression : public AST_Node
    {
        public:
            virtual const Type *type() const = 0;
    };

    /**
     *
     */
    class BinaryExpression : public Expression
    {
        public:
            BinaryExpression(Expression *lhs, Expression *rhs)
                : lhs_(lhs), rhs_(rhs) {}

            ~BinaryExpression() {
                delete lhs_;
                delete rhs_;
            }

            Expression *lhs() {
                return lhs_;
            }
            Expression *rhs() {
                return rhs_;
            }
        protected:
            Expression *lhs_;
            Expression *rhs_;
        private:
            BinaryExpression(const BinaryExpression &) = delete;
            BinaryExpression &operator=(const BinaryExpression &) = delete;
    };

    /**
     *
     */
    class UnaryExpression : public Expression
    {

    };

    /**
     *
     */
    class BinaryBoolExpression : public BinaryExpression
    {
        public:
            BinaryBoolExpression(Expression *lhs, Expression *rhs)
                : BinaryExpression(lhs, rhs),
                  type_(TypeFactory::get("bool")) {
                assert(lhs->type() == type_);
                assert(rhs->type() == type_);
            }

            virtual void accept(AST_Visitor &) = 0;
            virtual const Type *type() const {
                return type_;
            }
        private:
            BinaryBoolExpression(const BinaryBoolExpression &) = delete;
            BinaryBoolExpression &operator=(
                const BinaryBoolExpression &) = delete;
            const Type *type_;
    };

    /** And class
     *
     * The class only accepts left and right hand sides that are of type bool
     */
    class And : public BinaryBoolExpression
    {
        public:
            And(Expression *lhs, Expression *rhs)
                : BinaryBoolExpression(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    /** Or class
     *
     * The class only accepts left and right hand sides that are of type bool
     */
    class Or : public BinaryBoolExpression
    {
        public:
            Or(Expression *lhs, Expression *rhs)
                : BinaryBoolExpression(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    /** Boolean not
     *
     * The class only accepts expressions of type bool
     */
    class Not : public UnaryExpression
    {
        public:
            Not(Expression *e)
                : UnaryExpression(), type_(TypeFactory::get("bool")),
                  expression_(e) {
                assert(e->type() == type_);
            }

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return type_;
            }

            Expression *expression() {
                return expression_;
            }
        private:
            Not(const Not &) = delete;
            Not &operator=(const Not &) = delete;
            const Type *type_;
            Expression *expression_;
    };

    /** Boolean equivalence class
     *
     * The class only accepts left and right hand sides that are of type bool
     */
    class BoolEquals : public BinaryBoolExpression
    {
        public:
            BoolEquals(Expression *lhs, Expression *rhs)
                : BinaryBoolExpression(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    /** Integer comparator
     *
     * Expressions of type int [cmp] int -> bool
     */
    class IntCompare : public BinaryExpression
    {
        public:
            IntCompare(Expression *lhs, Expression *rhs)
                : BinaryExpression(lhs, rhs),
                  type_(TypeFactory::get("bool")) {
                assert(lhs->type() == TypeFactory::get("int"));
                assert(rhs->type() == TypeFactory::get("int"));
            }

            virtual void accept(AST_Visitor &) = 0;
            virtual const Type *type() const {
                return type_;
            }
        private:
            IntCompare(const IntCompare &) = delete;
            IntCompare &operator=(const IntCompare &) = delete;

            const Type *type_;
    };

    class LessThan : public IntCompare
    {
        public:
            LessThan(Expression *lhs, Expression *rhs)
                : IntCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class LessThanOrEqual : public IntCompare
    {
        public:
            LessThanOrEqual(Expression *lhs, Expression *rhs)
                : IntCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class GreaterThan : public IntCompare
    {
        public:
            GreaterThan(Expression *lhs, Expression *rhs)
                : IntCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class GreaterThanOrEqual : public IntCompare
    {
        public:
            GreaterThanOrEqual(Expression *lhs, Expression *rhs)
                : IntCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class Equals : public IntCompare
    {
        public:
            Equals(Expression *lhs, Expression *rhs)
                : IntCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    /**
     *
     */
    class BinaryIntExpression : public BinaryExpression
    {
        public:
            BinaryIntExpression(Expression *lhs, Expression *rhs)
                : BinaryExpression(lhs, rhs),
                  type_(TypeFactory::get("int")) {
                assert(lhs->type() == type_);
                assert(rhs->type() == type_);
            }

            virtual void accept(AST_Visitor &) = 0;
            virtual const Type *type() const {
                return type_;
            }
        private:
            BinaryIntExpression(const BinaryIntExpression &) = delete;
            BinaryIntExpression &operator=(
                const BinaryIntExpression &) = delete;

            const Type *type_;
    };

    /** Plus class
     *
     * Represents integer addition
     */
    class Plus : public BinaryIntExpression
    {
        public:
            Plus(Expression *lhs, Expression *rhs)
                : BinaryIntExpression(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    /** Minus class
     *
     * Represents integer subtraction
     */
    class Minus : public BinaryIntExpression
    {
        public:
            Minus(Expression *lhs, Expression *rhs)
                : BinaryIntExpression(lhs, rhs) {}
            virtual void accept(AST_Visitor &);
    };

    /** Times class
     *
     * Represents integer multiplication
     */
    class Times : public BinaryIntExpression
    {
        public:
            Times(Expression *lhs, Expression *rhs)
                : BinaryIntExpression(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class StringCompare : public BinaryExpression
    {
        public:
            StringCompare(Expression *lhs, Expression *rhs)
                : BinaryExpression(lhs, rhs),
                  type_(TypeFactory::get("bool")) {
                assert(lhs->type() == TypeFactory::get("string"));
                assert(rhs->type() == TypeFactory::get("string"));
            }

            virtual void accept(AST_Visitor &) = 0;
            virtual const Type *type() const {
                return type_;
            }
        private:
            StringCompare(const StringCompare &) = delete;
            StringCompare &operator=(const StringCompare &) = delete;

            const Type *type_;
    };

    class StringLessThan : public StringCompare
    {
        public:
            StringLessThan(Expression *lhs, Expression *rhs)
                : StringCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class StringLessThanOrEqual : public StringCompare
    {
        public:
            StringLessThanOrEqual(Expression *lhs, Expression *rhs)
                : StringCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class StringGreaterThan : public StringCompare
    {
        public:
            StringGreaterThan(Expression *lhs, Expression *rhs)
                : StringCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class StringGreaterThanOrEqual : public StringCompare
    {
        public:
            StringGreaterThanOrEqual(Expression *lhs, Expression *rhs)
                : StringCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    class StringEquals : public StringCompare
    {
        public:
            StringEquals(Expression *lhs, Expression *rhs)
                : StringCompare(lhs, rhs) {}

            virtual void accept(AST_Visitor &);
    };

    /** String repetition class
     *
     *
     */
    class StringRepeat : public BinaryExpression
    {
        public:
            StringRepeat(Expression *string, Expression *mult)
                : BinaryExpression(string, mult),
                  type_(TypeFactory::get("string")) {
                assert(string->type() == type_);
                assert(mult->type() == TypeFactory::get("int"));
            }

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return type_;
            }
        private:
            StringRepeat(const StringRepeat &) = delete;
            StringRepeat &operator=(const StringRepeat &) = delete;

            const Type *type_;
    };

    /** String concatenation class
     *
     *
     */
    class StringConcat : public BinaryExpression
    {
        public:
            StringConcat(Expression *lhs, Expression *rhs)
                : BinaryExpression(lhs, rhs),
                  type_(TypeFactory::get("string")) {
                assert(lhs->type() == type_);
                assert(rhs->type() == type_);
            }

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return type_;
            }
        private:
            StringConcat(const StringConcat &) = delete;
            StringConcat &operator=(const StringConcat &) = delete;

            const Type *type_;
    };

    /** List concatenation class
     *
     *
     */
    class ListConcat : public BinaryExpression
    {
        public:
            ListConcat(Expression *lhs, Expression *rhs)
                : BinaryExpression(lhs, rhs),
                  type_(lhs->type()) {
                assert(lhs->type() == rhs->type());
                assert(lhs->type()->list() != nullptr);
            }

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return type_;
            }
        private:
            ListConcat(const ListConcat &) = delete;
            ListConcat &operator=(const ListConcat &) = delete;

            const Type *type_;
    };

    /**
     *
     */
    class Constant : public UnaryExpression
    {
        public:
            Constant(ConstantData *d)
                : data_(d) {}

            ~Constant() {
                delete data_;
            }

            ConstantData *data() {
                return data_;
            }

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return data_->type();
            }
        private:
            Constant(const Constant &) = delete;
            Constant &operator=(const Constant &) = delete;

            ConstantData *data_;
    };

    /**
     *
     */
    class SymbolRef : public UnaryExpression
    {
        public:
            SymbolRef(symbol::Symbol *s)
                : symbol_(s) {}

            ~SymbolRef() {

            }

            symbol::Symbol *symbol() {
                return symbol_;
            }

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return symbol_->get_type();
            }
        private:
            SymbolRef(const SymbolRef &) = delete;
            SymbolRef &operator=(const SymbolRef &) = delete;

            symbol::Symbol *symbol_;
    };

    class FieldRef : public UnaryExpression
    {
        public:
            FieldRef(Expression *r, const string &f)
                : record_(r), field_(f) {}

            ~FieldRef() {
                delete record_;
            }

            Expression *record() {
                return record_;
            }
            string field() {
                return field_;
            }

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return record_->type()->dot(field_);
            }
        private:
            FieldRef(const FieldRef &) = delete;
            FieldRef &operator=(const FieldRef &) = delete;

            Expression *record_;
            string field_;
    };

    /**
     *
     */
    struct ExpressionList
    {
        public:
            ExpressionList(Expression *e, ExpressionList *n = nullptr)
                : expression(e), next(n) {}

            ~ExpressionList() {
                delete expression;
                if (next)
                    delete next;
            }

            Expression *expression;
            ExpressionList *next;
        private:
            ExpressionList(const ExpressionList &) = delete;
            ExpressionList &operator=(const ExpressionList &) = delete;
    };

    /**
     *
     */
    class MethodCall : public UnaryExpression
    {
        public:
            MethodCall(Expression *e, const TypeMethod &m,
                       ExpressionList *a)
                : expression_(e), method_(m), args_(a) {}

            virtual void accept(AST_Visitor &);
            virtual const Type *type() const {
                return method_.return_type();
            }

            Expression *expression() {
                return expression_;
            }
            TypeMethod method() {
                return method_;
            }
            ExpressionList *arguments() {
                return args_;
            }
        private:
            MethodCall(const MethodCall &) = delete;
            MethodCall &operator=(const MethodCall &) = delete;

            Expression *expression_;
            TypeMethod method_;
            ExpressionList *args_;
    };

    /**
     *
     */
    class List : public UnaryExpression
    {
        public:
            List(const ListType *t)
                : type_(t), elems_(nullptr) {}

            ~List() {
                if (elems_)
                    delete elems_;
            }

            void set_elements(ExpressionList *e) {
                elems_ = e;
            }
            ExpressionList *elements() {
                return elems_;
            }

            virtual void accept(AST_Visitor &);
            virtual const ListType *type() const {
                return type_;
            }
        private:
            List(const List &) = delete;
            List &operator=(const List &) = delete;

            const ListType *type_;
            ExpressionList *elems_;
    };

    /**
     *
     */
    class Statement : public AST_Node
    {

    };

    /**
     *
     */
    class Conditional : public Statement
    {
        public:
            Conditional(If *if_node, Elif *elif_nodes, Else *else_node)
                : if_(if_node), elifs_(elif_nodes), else_(else_node) {}

            virtual void accept(AST_Visitor &);

            If *if_node() {
                return if_;
            }
            Elif *elif_nodes() {
                return elifs_;
            }
            Else *else_node() {
                return else_;
            }
        private:
            Conditional(const Conditional &) = delete;
            Conditional &operator=(const Conditional &) = delete;

            If *if_;
            Elif *elifs_;
            Else *else_;
    };

    /** Scope class
     *
     * A scope objects holds statements and an associated symbol table
     */
    class Scope : public Statement
    {
        public:
            Scope(symbol::SymbolTable *t, Statements *s)
                : table_(t), statements_(s) {}

            void set_statements(Statements *s) {
                statements_ = s;
            }

            Statements *statements() {
                return statements_;
            }

            symbol::SymbolTable *table() {
                return table_;
            }

            virtual void accept(AST_Visitor &) = 0;
        private:
            Scope(const Scope &) = delete;
            Scope &operator=(const Scope &) = delete;

            symbol::SymbolTable *table_;
            Statements *statements_;
    };

    /** ForEach class
     *
     * ForEach represents a for each statement in the language:
     *   ~~~
     *   % for [identifier] in [expression]
     *     [statements]
     *   % endfor
     *   ~~~
     *
     * @details The loop variable can be modified in the statements, therefore
     * the backend must ensure that any eventual modification doesn't mess up
     * the loop.
     */
    class ForEach : public Scope
    {
        public:
            ForEach(symbol::Variable *sy, Expression *e,
                    symbol::SymbolTable *ft,
                    symbol::SymbolTable *t)
                : Scope(t, nullptr), expression_(e), for_table_(ft),
                  variable_(sy) {}

            Expression *expression() {
                return expression_;
            }
            symbol::Symbol *variable() {
                return variable_;
            }

            virtual void accept(AST_Visitor &);
        private:
            ForEach(const ForEach &) = delete;
            ForEach &operator=(const ForEach &) = delete;

            Expression *expression_;
            symbol::SymbolTable *for_table_;
            symbol::Variable *variable_;
    };

    /** ForEachEnum class
     *
     * ForEachEnum represents an enumerated for each statement in the language:
     *   ~~~
     *   % for [index-identifier], [value-identifier] in [expression]
     *     [statements]
     *   % endfor
     *   ~~~
     *
     * @details The loop variables can be modified in the statements, therefore
     * the backend must ensure that any eventual modification doesn't mess up
     * the loop.
     */
    class ForEachEnum : public Scope
    {
        public:
            ForEachEnum(symbol::Variable *i,
                        symbol::Variable *v, Expression *e,
                        symbol::SymbolTable *ft,
                        symbol::SymbolTable *t)
                : Scope(t, nullptr), expression_(e), for_table_(ft),
                  index_(i), value_(v) {}

            Expression *expression() {
                return expression_;
            }

            symbol::Variable *index() {
                return index_;
            }

            symbol::Variable *value() {
                return value_;
            }

            virtual void accept(AST_Visitor &);
        private:
            ForEachEnum(const ForEachEnum &) = delete;
            ForEachEnum &operator=(const ForEachEnum &) = delete;

            Expression *expression_;
            symbol::SymbolTable *for_table_;
            symbol::Variable *index_;
            symbol::Variable *value_;
    };

    /**
     *
     */
    class If : public Scope
    {
        public:
            If(Expression *c, symbol::SymbolTable *t)
                : Scope(t, nullptr), condition_(c) {}

            void set_condition(Expression *c) {
                condition_ = c;
            }

            Expression *condition() {
                return condition_;
            }

            virtual void accept(AST_Visitor &);
        private:
            If(const If &) = delete;
            If &operator=(const If &) = delete;

            Expression *condition_;
    };

    /**
     *
     */
    class Elif : public Scope
    {
        public:
            Elif(Expression *c, symbol::SymbolTable *t)
                : Scope(t, nullptr), condition_(c), next_() {}

            void set_condition(Expression *c) {
                condition_ = c;
            }
            void set_next(Elif *n) {
                next_ = n;
            }

            Expression *condition() {
                return condition_;
            }
            Elif *next() {
                return next_;
            }

            virtual void accept(AST_Visitor &);
        private:
            Elif(const Elif &) = delete;
            Elif &operator=(const Elif &) = delete;

            Expression *condition_;
            Elif *next_;
    };

    /**
     *
     */
    class Else : public Scope
    {
        public:
            Else(symbol::SymbolTable *t) : Scope(t, nullptr) {}

            virtual void accept(AST_Visitor &);
    };

    /** Raw text
     *
     * The Text class holds raw text (in UTF-8 format)
     *
     */
    class Text : public Statement
    {
        public:
            Text(const string &s)
                : text_(s) {}

            virtual void accept(AST_Visitor &);

            string text() const {
                return text_;
            }
        private:
            string text_;
    };

    /**
     *
     */
    class InlinedExpression : public Statement
    {
        public:
            InlinedExpression(Expression *e)
                : expression_(e) {}

            ~InlinedExpression() {
                delete  expression_;
            }

            Expression *expression() {
                return expression_;
            }

            virtual void accept(AST_Visitor &);
        private:
            InlinedExpression(const InlinedExpression &) = delete;
            InlinedExpression &operator=(
                const InlinedExpression &) = delete;

            Expression *expression_;
    };

    /** TODO
     *
     */
    class VariableStatement : public Statement
    {
    };

    /** TODO
     *
     */
    struct VariableList : public Statement
    {
            VariableList(VariableStatement *s, VariableList *n)
                : statement(s), next(n) {}

            ~VariableList() {
                delete statement;
                if (next)
                    delete next;
            }

            VariableStatement *statement;
            VariableList *next;

            virtual void accept(AST_Visitor &);
        private:
            VariableList(const VariableList &) = delete;
            VariableList &operator=(const VariableList &) = delete;
    };

    /** TODO
     *
     */
    class VariableAssignment : public VariableStatement
    {
        public:
            VariableAssignment(symbol::Variable *v, Expression *e)
                : variable_(v), expression_(e) {}

            ~VariableAssignment() {
                delete expression_;
            }

            symbol::Variable *variable() {
                return variable_;
            }

            Expression *expression() {
                return expression_;
            }

            virtual void accept(AST_Visitor &);
        private:
            VariableAssignment(const VariableAssignment &) = delete;
            VariableAssignment &operator=(
                const VariableAssignment &) = delete;

            symbol::Variable *variable_;
            Expression *expression_;
    };

    /** TODO
    *
    */
    class VariableDeclaration : public VariableStatement
    {
        public:
            VariableDeclaration(symbol::Variable *v, Expression *e)
                : variable_(v), assignment_(new VariableAssignment(v, e)) {}

            ~VariableDeclaration() {
                delete assignment_;
            }

            symbol::Variable *variable() {
                return variable_;
            }

            VariableAssignment *assignment() {
                return assignment_;
            }

            virtual void accept(AST_Visitor &);
        private:
            VariableDeclaration(const VariableDeclaration &) = delete;
            VariableDeclaration &operator=(
                const VariableDeclaration &) = delete;

            symbol::Variable *variable_;
            VariableAssignment *assignment_;
    };

    /** Statements class
     *
     * Statements represents a list of statements. A Statements object holds a
     * pointer to the next statements object.
     */
    class Statements : public AST_Node
    {
        public:
            Statements(Statement *s, Statements *p = nullptr)
                : statement_(s), next_(p) {}

            ~Statements() {
                delete statement_;
                if (next_)
                    delete next_;
            }

            virtual void accept(AST_Visitor &);

            Statement *statement() {
                return statement_;
            }
            Statements *next() {
                return next_;
            }
        private:
            Statements(const Statements &) = delete;
            Statements &operator=(const Statements &) = delete;

            Statement *statement_;
            Statements *next_;
    };

    /**
     *
     */
    class AST_Visitor
    {
        public:
            virtual void visit(And *) = 0;
            virtual void visit(Or *) = 0;
            virtual void visit(Not *) = 0;
            virtual void visit(BoolEquals *) = 0;
            virtual void visit(LessThan *) = 0;
            virtual void visit(LessThanOrEqual *) = 0;
            virtual void visit(GreaterThan *) = 0;
            virtual void visit(GreaterThanOrEqual *) = 0;
            virtual void visit(Equals *) = 0;
            virtual void visit(Plus *) = 0;
            virtual void visit(Minus *) = 0;
            virtual void visit(Times *) = 0;
            virtual void visit(StringLessThan *) = 0;
            virtual void visit(StringLessThanOrEqual *) = 0;
            virtual void visit(StringGreaterThan *) = 0;
            virtual void visit(StringGreaterThanOrEqual *) = 0;
            virtual void visit(StringEquals *) = 0;
            virtual void visit(StringRepeat *) = 0;
            virtual void visit(StringConcat *) = 0;
            virtual void visit(ListConcat *) = 0;
            virtual void visit(Constant *) = 0;
            virtual void visit(MethodCall *) = 0;
            virtual void visit(SymbolRef *) = 0;
            virtual void visit(FieldRef *) = 0;
            virtual void visit(List *) = 0;

            virtual void visit(Statements *) = 0;

            virtual void visit(Conditional *) = 0;
            virtual void visit(ForEach *) = 0;
            virtual void visit(ForEachEnum *) = 0;
            virtual void visit(If *) = 0;
            virtual void visit(Elif *) = 0;
            virtual void visit(Else *) = 0;
            virtual void visit(Text *) = 0;
            virtual void visit(InlinedExpression *) = 0;
            /* TODO */
            virtual void visit(VariableList *) {}
            virtual void visit(VariableDeclaration *) {}
            virtual void visit(VariableAssignment *) {}
    };
}

#endif
