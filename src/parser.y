%{
#include <algorithm>
#include <iostream>

#include "ast.hpp"
#include "symbol.hpp"

using namespace constant;
using namespace symbol;

// TODO: fix Single/Scalar/Primtive things

SymbolTable symbol_table;

/* constant_list is used by list_constant to hold the list elements.
  This is used instead of a AST approach and is ok since only one list will be
  handled at a time (since a list can only contain primitives and records)
  same reasoning for constant_record */
std::vector<SingleConstantData *> constant_list;
std::vector<PrimitiveConstantData *> constant_record;

/* same reasoning as above */
std::vector<Param *> param_list;

RecordType::field_vector record_members;

extern int yylex();
void yyerror(const char *);
void vyyerror(const char *, ...);

template<class T>
ast::BinaryExpression *create_bool_binary(ast::Expression *lhs,
   ast::Expression *rhs)
{
    if (lhs->type() != TypeFactory::get("bool"))
        throw DifferentTypesError(lhs->type(), TypeFactory::get("bool"));
    if (rhs->type() != TypeFactory::get("bool"))
        throw DifferentTypesError(rhs->type(), TypeFactory::get("bool"));
    return new T(lhs, rhs);
}

ast::BinaryExpression *create_plus_binary(ast::Expression *lhs,
   ast::Expression *rhs)
{
        if (lhs->type() == rhs->type()) {
            if (lhs->type() == TypeFactory::get("int"))
                return new ast::Plus(lhs, rhs);
            else if (lhs->type() == TypeFactory::get("string"))
                return new ast::StringConcat(lhs, rhs);
            else if (lhs->type()->list())
                return new ast::ListConcat(lhs, rhs);
        }
        throw InvalidTypeError("Can't apply '+' operand on " +
            lhs->type()->str()  + " and " + rhs->type()->str());
}

ast::BinaryExpression *create_minus_binary(ast::Expression *lhs,
   ast::Expression *rhs)
{
        if (lhs->type() == rhs->type() &&
            lhs->type() == TypeFactory::get("int"))
            return new ast::Minus(lhs, rhs);
        throw InvalidTypeError("Can't apply '-' operand on " +
            lhs->type()->str()  + " and " + rhs->type()->str());
}

%}

%error-verbose

%code requires {
	#include "ast.hpp"
	#include "constant.hpp"
	#include "symbol.hpp"
	#include "type.hpp"

	using namespace constant;
	using namespace symbol;
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
    SingleConstantData *single_const;
    PrimitiveConstantData *primitive_const;
    RecordConstantData *record_const;
    ListConstantData *list_const;
    Argument *argument;
    Param *param;

    ast::Statements *statements;
    ast::Statement *statement;
    ast::Expression *expression;
}

%token END 0 "end of file"
%token ARGUMENT "argument"
%token RECORD "record"
%token SEPARATOR "%%"
%token CONTROL "%"
%token<string> IDENTIFIER "identifier"
%token FOR "for" IN "in" ENDFOR "endfor"
%token IF "if" ELIF "elif" ELSE "else" ENDIF "endif"
%token AND "and" OR "or" NOT "not"
%token<string> TEXT

%token<boolean> BOOL "bool constant"
%token<integer> INT "integer constant"
%token<string> STRING "string constant"

%token<string> LIST

%type<constant> constant
%type<single_const> single_constant
%type<primitive_const> primitive_constant
%type<record_const> record_constant
%type<list_const> list_constant

%type<argument> arg

%type<param> param

%type<type> type
%type<stype> single_type
%type<ltype> list_type

%type<statements> statements

%type<statement> statement
%type<statement> text
%type<statement> conditional
%type<statement> inlined
%type<statement> loop

%type<expression> expression

%left OR
%left AND
%left '+' '-'

%start file

%%

file
    : header_block SEPARATOR body_block
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
    | record_def
    ;

arg
    : ARGUMENT type IDENTIFIER '{' header_item_params '}'
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

record_def
    : RECORD IDENTIFIER '{' record_def_members '}'
    {
        const Type *t = TypeFactory::get($2);

        if (t != nullptr) {
            vyyerror("Multiple definitions of type '%s'", $2);
            YYERROR;
        }

        TypeFactory::add_record($2, record_members);
        record_members.clear();

        free($2);
    }

record_def_members
    : record_def_members record_member { }
    | record_member { }
    ;

record_member
    : single_type IDENTIFIER ';'
    {
        const PrimitiveType *p = $1 ? $1->primitive() : nullptr;

        if (p == nullptr) {
            vyyerror("A record can only hold primitive types", $2);
            YYERROR;
        }

        auto it = find_if(record_members.begin(), record_members.end(),
            [&] (const RecordField &r) { return r.name == $2; });

        if (it != record_members.end()) {
            vyyerror("Multiple definitions of field '%s'", $2);
            YYERROR;
        }

        record_members.push_back({ $2, p});

        free($2);
    }
    | list_type IDENTIFIER ';'
    {
        vyyerror("A record can only hold primitive types", $2);
        YYERROR;
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
    : IDENTIFIER '=' constant ';'
    {
        $$ = new Param($1, $3);
        free($1);
    }
    ;

body_block
    : statements
    {
        ast::AST_Printer p;

        std::cout << $1 << std::endl;
        if ($1)
            $1->accept(p);
    }
    |
    ;

statements
    : statements statement { $$ = new ast::Statements($2, $1); }
    | statement { $$ = new ast::Statements($1); }
    ;

statement
    : text { $$ = $1; }
    | conditional { $$ = $1; }
    | loop { $$ = $1; }
    | inlined { $$ = $1; }
    /* TODO: handle empty */
    ;

text
    : TEXT
    {
        $$ = new ast::Text($1);
        free($1);
    }
    ;

conditional
    : { /* TODO */ }
    ;

loop
    : CONTROL FOR IDENTIFIER IN expression statements CONTROL ENDFOR
    {
        std::cout << "for " << $3 << " in " << std::endl;

        ast::AST_Printer p;
        $5->accept(p);
        /* TODO */
    }
    ;

inlined
    : { /* TODO */ }
    ;

 /* TODO: precedence */
expression
    : expression AND expression
    {
        try {
            $$ = create_bool_binary<ast::And>($1, $3);
        } catch (const DifferentTypesError &e) {
            vyyerror("Invalid type for 'and' operator (%s)",
                e.what());
            YYERROR;
        }
    }
    | expression OR expression
    {
        try {
            $$ = create_bool_binary<ast::Or>($1, $3);
        } catch (const DifferentTypesError &e) {
            vyyerror("Invalid type for 'or' operator (%s)",
                e.what());
            YYERROR;
        }
    }
    | expression '+' expression
    {
        try {
            $$ = create_plus_binary($1, $3);
        } catch (const InvalidTypeError &e) {
            vyyerror("%s", e.what());
            YYERROR;
        }
    }
    | expression '-' expression
    {
        try {
            $$ = create_minus_binary($1, $3);
        } catch (const InvalidTypeError &e) {
            vyyerror("Unknown type '%s'", $1);
            YYERROR;
        }
    }
    | expression '*' expression
    {
        try {
            /*$$ = create_times($1, $3);*/
        } catch (const InvalidTypeError &e) {

        }
        /* TODO: $1->type() == string, 2->type() == int => string repeat */
        /* TODO: $1->type() == int, 2->type() == string => string repeat */
        /* TODO: $1->type() == int, 2->type() == int => * */
    }
    | constant
    {
        $$ = new ast::Constant($1);
    }
    | IDENTIFIER
    {
        /* TODO */
    }
    | '(' expression ')'
    {
        $$ = $2;
    }

constant
    : single_constant { $$ = $1; }
    | list_constant { $$ = $1; }
    ;

single_constant
    : primitive_constant { $$ = $1; }
    | record_constant { $$ = $1; }

primitive_constant
    : BOOL { $$ = new BoolConstantData($1); }
	| INT { $$ = new IntConstantData($1); }
	| STRING { $$ = new StringConstantData($1); }
	;

record_constant
    : IDENTIFIER '{' record_values '}'
    {
        const Type *t = TypeFactory::get($1);

        if (t == nullptr) {
            vyyerror("Unknown type '%s'", $1);
            YYERROR;
        }

        const RecordType *p = t->record();

        if (p == nullptr) {
            vyyerror("Expected a record (got '%s')", t->str().c_str());
        }

        try {
            validate_field_types(p, constant_record);
        } catch (const UnevenNoOfFieldsException &e) {
            vyyerror("%s", e.what());
            YYERROR;
        } catch (const DifferentTypesError &e) {
            vyyerror("Invalid type for field  (%s)", e.what()); // TODO improve
            YYERROR;
        }

        $$ = new RecordConstantData(p, constant_record);

        constant_record.clear();
    }

record_values
    : record_values ',' primitive_constant { constant_record.push_back($3); }
    | primitive_constant { constant_record.push_back($1); }
    ;

type
    : single_type { $$ = $1; }
    | list_type { $$ = $1; }

single_type
    : IDENTIFIER
    {
        const Type *t = TypeFactory::get($1);

        $$ = t ? t->single() : nullptr;

        if ($$ == nullptr) {
            vyyerror("Unknown type '%s'", $1);
            YYERROR;
        }
    }
    ;

list_type
    : LIST
    {
        const Type *t = TypeFactory::get($1);

        $$ = t ? t->list() : nullptr;

        if ($$ == nullptr) {
            vyyerror("Unknown type '%s'", $1);
            YYERROR;
        }
    }

list_constant
    : '[' list_values ']'
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
    : list_values ',' single_constant { constant_list.push_back($3); }
    | single_constant { constant_list.push_back($1); }
    ;

%%
