#include <iostream>

//extern Arguments *args;
//extern Statements *body;

using namespace std;

extern int yyparse();

extern FILE *yyin;

int main(int argc, char **argv)
{
	yyparse();

	return 0;
}
