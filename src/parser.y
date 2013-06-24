%{
#include <iostream>

extern int yylex();
void yyerror(const char *);
%}

%union {
	char *string;
	bool boolean;
	int integer;
	bool is_list;
}

%token T_ARGUMENT
%token T_SEPARATOR
%token T_CONTROL
%token T_L_BRACE T_R_BRACE
%token T_ASSIGNMENT
%token<string> T_IDENTIFIER
%token T_FOR T_IN T_ENDFOR
%token T_IF T_ELIF T_ELSE T_ENDIF
%token T_BOOL T_INT T_STRING
%token T_BOOL_TYPE T_INT_TYPE T_STRING_TYPE

%start file

%%

file :
     ;

%%

void yyerror(const char *s)
{
	std::cerr << "Error: " << s << "\n";
}
