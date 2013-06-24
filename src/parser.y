%{
#include <iostream>


extern int yylex();
void yyerror(const char *);
%}

%error-verbose

%code requires {
	#include "type.hpp"
}

%union {
	char *string;
	bool boolean;
	int integer;
	bool is_list;
	Type type;
}

%token END 0 "end of file"
%token ARGUMENT
%token SEPARATOR
%token CONTROL
%token L_BRACE R_BRACE
%token ASSIGNMENT
%token<string> IDENTIFIER
%token FOR IN ENDFOR
%token IF ELIF ELSE ENDIF

%token<boolean> BOOL
%token<integer> INT
%token<string> STRING

%token<type> TYPE

%start file

%%

file : header_block
     ;

header_block : header_block header_item { }
	     | header_item {}
	     |
	     ;

header_item : arg
	    ;

arg : ARGUMENT TYPE IDENTIFIER L_BRACE R_BRACE {
    std::cout << "type: " << type_to_str($2) << std::endl;
    std::cout << "identifier: " << $3 << std::endl;
}

%%

void yyerror(const char *s)
{
	std::cerr << "Error: " << s << "\n";
}
