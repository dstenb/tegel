%{
#include <iostream>

extern int yylex();
void yyerror(const char *);
%}

%union {
}

%start file

%%

file :
     ;

%%

void yyerror(const char *s)
{
	std::cerr << "Error: " << s << "\n";
}
