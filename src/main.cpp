#include <iostream>

//extern Arguments *args;
//extern Statements *body;

#include "type.hpp"

using namespace std;
using type::TypeFactory;

extern void setup_symbol_table();
extern int yyparse();

extern FILE *yyin;

int main(int argc, char **argv)
{
	setup_symbol_table();
	yyparse();

	cout << "Types that are defined:\n";
	TypeFactory::print(cout);

	return 0;
}
