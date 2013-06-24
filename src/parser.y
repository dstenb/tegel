%{
#include <iostream>

extern int yylex();
void yyerror(const char *);
%}

%union {
}

%token T_BODY T_CONTROL
%token T_L_BRACE T_R_BRACE
%token T_ASSIGNMENT
%token T_IDENTIFIER
%token T_FOR T_IN T_ENDFOR
%token T_IF T_ELIF T_ELSE T_ENDIF

%start file

%%

file :
     ;

%%

void yyerror(const char *s)
{
	std::cerr << "Error: " << s << "\n";
}
