%{
#include <iostream>


extern int yylex();
void yyerror(const char *);
%}

%error-verbose

%code requires {
	#include "constant.hpp"
	#include "symbol.hpp"
	#include "type.hpp"
}

%union {
	char *string;
	bool boolean;
	int integer;
	bool is_list;
	Type type;
	ConstantData *constant;
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

%type<constant> constant
%type<constant> scalar_constant
%type<constant> list_constant

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

header_item :
	    arg
	    {
	      // TODO: add to argument vector
	      // TODO: add to symbol table
	    }
	    ;

arg : ARGUMENT TYPE IDENTIFIER L_BRACE header_item_params R_BRACE  {
    // TODO:
    // $$ = new Argument($3, $2);
    // try {
    // $$->add_parameters($5)
    // } catch(const ParameterException &e) {
    //
    // }
    // Free data (e.g. $3);
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
	std::cout << "param(" << $1 << "=" << *$3 << ")\n";
}

constant : scalar_constant { $$ = $1; }
	 | list_constant { $$ = $1; }

scalar_constant : BOOL { $$ = new BoolConstantData($1); }
		| INT { $$ = new IntConstantData($1); }
		| STRING { $$ = new StringConstantData($1); }
		;

list_constant : L_BRACKET list_values R_BRACKET { }
	      | L_BRACKET R_BRACKET {}
	      ;

list_values : list_values scalar_constant { }
	    | scalar_constant { }
	    ;

%%
