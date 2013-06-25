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
%token ARGUMENT "argument"
%token SEPARATOR
%token CONTROL
%token L_BRACE "{" R_BRACE "}" SEMI_COLON ";"
%token L_BRACKET "[" R_BRACKET "]" ASSIGNMENT "="
%token<string> IDENTIFIER "identifier"
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

header_block : header_block_p { }
	     |
	     ;

header_block_p : header_block_p header_item { }
	       | header_item { }
	       ;

header_item : arg
	    ;

arg : ARGUMENT TYPE IDENTIFIER L_BRACE header_item_params R_BRACE  {
    std::cout << "type: " << type_to_str($2) << std::endl;
    std::cout << "identifier: " << $3 << std::endl;
}

header_item_params : header_item_params_p { }
		   |
		   ;

header_item_params_p : header_item_params_p param { }
		   | param { }
		   ;

param : IDENTIFIER ASSIGNMENT constant SEMI_COLON {
		   std::cout << "param(" << $1 << "=" << ")\n";
}

constant : scalar_constant { }
	 | list_constant { }

scalar_constant : BOOL { }
		| INT { }
		| STRING { }
		;

list_constant : L_BRACKET list_values R_BRACKET { }
	      | L_BRACKET R_BRACKET {}
	      ;

list_values : list_values scalar_constant { }
	    | scalar_constant { }
	    ;



%%
