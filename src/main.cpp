#include <iostream>

//extern Arguments *args;
//extern Statements *body;

#include "type.hpp"

using namespace std;

extern int yyparse();

extern FILE *yyin;

int main(int argc, char **argv)
{
	cout << yyparse() << endl;

	TypeFactory::print(cout);

	return 0;
}
