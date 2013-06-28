%{
#include <iostream>

#include "symbol.hpp"

// TODO: fix Single/Scalar/Primtive things

SymbolTable symbol_table;

/* constant_list is used by list_constant to hold the list elements.
  This is used instead of a AST approach and is ok since only one list will be
  handled at a time (since a list can only contain scalars) */
std::vector<SingleConstantData *> constant_list;

/* same reasoning as above */
std::vector<Param *> param_list;

extern int yylex();
void yyerror(const char *);
void vyyerror(const char *, ...);
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
	const Type *type;
	const SingleType *stype;
	const ListType *ltype;
	ConstantData *constant;
	SingleConstantData *scalar_const;
	ListConstantData *list_const;
	Argument *argument;
    Param *param;
}

%token END 0 "end of file"
%token ARGUMENT "argument"
%token SEPARATOR
%token CONTROL
%token L_BRACE "{" R_BRACE "}" SEMI_COLON ";" COMMA ","
%token L_BRACKET "[" R_BRACKET "]" ASSIGNMENT "="
%token<string> IDENTIFIER "identifier"
%token FOR IN ENDFOR
%token IF ELIF ELSE ENDIF

%token<boolean> BOOL
%token<integer> INT
%token<string> STRING

%token<string> LIST

%type<constant> constant
%type<scalar_const> scalar_constant
%type<list_const> list_constant

%type<argument> arg

%type<param> param

%type<type> type
%type<stype> single_type
%type<ltype> list_type

%start file

%%

file
    : header_block
    {
        symbol_table.print(std::cout);
    }
    ;

header_block
    : header_block_p { }
    |
    ;

header_block_p
    : header_block_p header_item { }
    | header_item { }
    ;

header_item
    : arg
    {
        // TODO: add to argument vector
        try {
            symbol_table.add($1);
        } catch(const SymTabAlreadyDefinedError &e) {
            vyyerror("Argument %s is already defined",
                $1->get_name().c_str());
            YYERROR;
        }
    }
    ;

arg
    : ARGUMENT type IDENTIFIER L_BRACE header_item_params R_BRACE
    {
        $$ = new Argument($3, $2);

        for (Param *p : param_list) {
            // TODO: $$->set(p);
            try {
                Param *op = $$->replace(p);
                if (op)
                    delete op;
            } catch (const ParamException &e) {
                vyyerror("%s", e.what());
                YYERROR;
            }
        }
        param_list.clear();
        free($3); // free identifier string
    }

header_item_params
    : header_item_params_p { }
    |
    ;

header_item_params_p
    : header_item_params_p param { param_list.push_back($2); }
    | param { param_list.push_back($1); }
    ;

param
    : IDENTIFIER ASSIGNMENT constant SEMI_COLON
    {
        $$ = new Param($1, $3);
        free($1);
    }
    ;

constant
    : scalar_constant { $$ = $1; }
    | list_constant { $$ = $1; }
    ;

scalar_constant
    : BOOL { $$ = new BoolConstantData($1); }
	| INT { $$ = new IntConstantData($1); }
	| STRING { $$ = new StringConstantData($1); }
	;

type
    : single_type { $$ = $1; }
    | list_type { $$ = $1; }

single_type
    : IDENTIFIER
    {
        $$ = dynamic_cast<const SingleType *>(TypeFactory::get($1));

        if ($$ == nullptr) {
            vyyerror("Unknown type %s", $1);
            YYERROR;
        }
    }
    ;

list_type
    : LIST
    {
        $$ = dynamic_cast<const ListType *>(TypeFactory::get($1));

        if ($$ == nullptr) {
            vyyerror("Unknown type %s", $1);
            YYERROR;
        }
    }

list_constant
    : L_BRACKET list_values R_BRACKET
    {
        $$ = new ListConstantData(TypeFactory::get_list(
            constant_list.front()->type()));

        try {
            for (SingleConstantData *d : constant_list)
                $$->add(d);
        } catch (const InvalidTypeError &e) {
            yyerror(e.what());
            YYERROR;
        } catch (const DifferentTypesError &e) {
            vyyerror("A list can only hold items of same type (%s)",
                e.what());
            YYERROR;
        }

        constant_list.clear();
    }
    | list_type
    {
        $$ = new ListConstantData($1);
    }
    ;

list_values
    : list_values COMMA scalar_constant { constant_list.push_back($3); }
    | scalar_constant { constant_list.push_back($1); }
    ;

%%
