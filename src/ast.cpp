#include "ast.hpp"

namespace ast {

/* Accept function macro, quite ugly, quite smooth */
#define GENERATE_ACCEPT(x) void x::accept(AST_Visitor &v) { v.visit(this); }

GENERATE_ACCEPT(And)
GENERATE_ACCEPT(Or)
GENERATE_ACCEPT(Plus)
GENERATE_ACCEPT(Minus)
GENERATE_ACCEPT(Times)
GENERATE_ACCEPT(StringConcat)
GENERATE_ACCEPT(ListConcat)
GENERATE_ACCEPT(Constant)
GENERATE_ACCEPT(FunctionCall)
GENERATE_ACCEPT(SymbolRef)

GENERATE_ACCEPT(Statements)

GENERATE_ACCEPT(For)
GENERATE_ACCEPT(If)
GENERATE_ACCEPT(Elif)
GENERATE_ACCEPT(Else)
GENERATE_ACCEPT(Text)
GENERATE_ACCEPT(InlinedExpression)

}
