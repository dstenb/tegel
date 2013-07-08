#include <iostream>
#include <vector>

#include "ast.hpp"
#include "type.hpp"

using namespace std;
using type::TypeFactory;

extern vector<symbol::Argument *> arguments;
extern ast::Statements *body;

extern void setup_symbol_table();
extern int yyparse();

extern FILE *yyin;

int main(int argc, char **argv)
{
	setup_symbol_table();
	yyparse();

        ast::AST_Printer p;
        if (body)
            body->accept(p);

	cout << "Types that are defined:\n";
	TypeFactory::print(cout);

	return 0;
}
