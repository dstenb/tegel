#include "ast.hpp"

namespace ast {

    /* Accept function macro, quite ugly, quite smooth */
#define GENERATE_ACCEPT(x) void x::accept(AST_Visitor &v) { v.visit(this); }

    GENERATE_ACCEPT(TernaryIf)
    GENERATE_ACCEPT(And)
    GENERATE_ACCEPT(Or)
    GENERATE_ACCEPT(Not)
    GENERATE_ACCEPT(BoolEquals)
    GENERATE_ACCEPT(LessThan)
    GENERATE_ACCEPT(LessThanOrEqual)
    GENERATE_ACCEPT(GreaterThan)
    GENERATE_ACCEPT(GreaterThanOrEqual)
    GENERATE_ACCEPT(Equals)
    GENERATE_ACCEPT(Plus)
    GENERATE_ACCEPT(Minus)
    GENERATE_ACCEPT(Times)
    GENERATE_ACCEPT(StringLessThan)
    GENERATE_ACCEPT(StringLessThanOrEqual)
    GENERATE_ACCEPT(StringGreaterThan)
    GENERATE_ACCEPT(StringGreaterThanOrEqual)
    GENERATE_ACCEPT(StringEquals)
    GENERATE_ACCEPT(StringRepeat)
    GENERATE_ACCEPT(StringConcat)
    GENERATE_ACCEPT(ListConcat)
    GENERATE_ACCEPT(Constant)
    GENERATE_ACCEPT(MethodCall)
    GENERATE_ACCEPT(SymbolRef)
    GENERATE_ACCEPT(FieldRef)
    GENERATE_ACCEPT(List)
    GENERATE_ACCEPT(Record)

    GENERATE_ACCEPT(Statements)
    GENERATE_ACCEPT(Conditional)
    GENERATE_ACCEPT(ForEach)
    GENERATE_ACCEPT(ForEachEnum)
    GENERATE_ACCEPT(If)
    GENERATE_ACCEPT(Elif)
    GENERATE_ACCEPT(Else)
    GENERATE_ACCEPT(Text)
    GENERATE_ACCEPT(InlinedExpression)
    GENERATE_ACCEPT(VariableList)
    GENERATE_ACCEPT(VariableDeclaration)
    GENERATE_ACCEPT(VariableAssignment)
    GENERATE_ACCEPT(Create)
}
