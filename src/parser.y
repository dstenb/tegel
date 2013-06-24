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
%token T_ARGUMENT
%token T_SEPARATOR
%token T_CONTROL
%token T_L_BRACE T_R_BRACE
%token T_ASSIGNMENT
%token<string> T_IDENTIFIER
%token T_FOR T_IN T_ENDFOR
%token T_IF T_ELIF T_ELSE T_ENDIF

%token<boolean> T_BOOL
%token<integer> T_INT
%token<string> T_STRING

%token<type> T_TYPE

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

arg : T_ARGUMENT T_TYPE T_IDENTIFIER T_L_BRACE T_R_BRACE {
    std::cout << "type: " << type_to_str($2) << std::endl;
}

%%

void yyerror(const char *s)
{
	std::cerr << "Error: " << s << "\n";
}
