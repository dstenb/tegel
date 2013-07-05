%{
#include <algorithm>
#include <iostream>

#include "ast.hpp"
#include "ast_factories.hpp"
#include "symbol.hpp"

using namespace constant;
using namespace symbol;

SymbolTable *root_table;
SymbolTable *current_table;

/* constant_list is used by the constant_list grammar rule to hold the list
  elements. This is used instead of a AST approach and is ok since only one
  list will be handled at a time (since a list can only contain primitives and
  records) same reasoning for constant_record */
std::vector<SingleConstantData *> constant_list;
std::vector<PrimitiveConstantData *> constant_record;

/* same reasoning as above */
std::vector<Param *> param_list;

RecordType::field_vector record_members;

extern int yylex();
void yyerror(const char *);
void vyyerror(const char *, ...);

%}

%error-verbose

%code requires {
	#include "ast.hpp"
    #include "ast_factories.hpp"
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

    ast::Scope *scope;
    ast::If *if_node;
    ast::Elif *elif_node;
    ast::Else *else_node;

    ast::List *list;
    ast::ExpressionList *expression_list;
}

%token END 0 "end of file"
%token ARGUMENT "argument"
%token RECORD "record"
%token SEPARATOR "%%"
%token<string> IDENTIFIER "identifier"
%token FOR "for" IN "in" ENDFOR "endfor"
%token IF "if" ELIF "elif" ELSE "else" ENDIF "endif"
%token AND "and" OR "or" NOT "not"
%token L_INLINE "{{" R_INLINE "}}"
%token<string> TEXT

%token<boolean> BOOL "bool constant"
%token<integer> INT "integer constant"
%token<string> STRING "string constant"

%token<string> LIST

%type<constant> constant
%type<single_const> single_constant
%type<primitive_const> primitive_constant
%type<record_const> record_constant
%type<list_const> constant_list

%type<argument> arg

%type<param> param

%type<type> type
%type<stype> single_type
%type<ltype> list_type

%type<statements> statements

%type<statement> statement text conditional control inlined
%type<scope> loop for_each
%type<if_node> if if_start
%type<elif_node> elif_start elifs elif
%type<else_node> else_start else

%type<expression> expression

%type<list> list
%type<expression_list> list_values

%left OR
%left AND
%left '+' '-'
%left '*'

%start file

%%

file
    : header_block SEPARATOR body_block
    {
        root_table->print(std::cout);
    }

/***************************************************************************
 *
 * Header related productions
 *
 ***************************************************************************/

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
            root_table->add($1);
        } catch(const SymTabAlreadyDefinedError &e) {
            stringstream sstr;
            root_table->lookup($1->get_name())->print(sstr);
            vyyerror("%s is already defined (as %s)\n",
                $1->get_name().c_str(), sstr.str().c_str());
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

constant_list
    : '[' constant_list_values ']'
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

constant_list_values
    : constant_list_values ',' single_constant { constant_list.push_back($3); }
    | single_constant { constant_list.push_back($1); }
    ;



/***************************************************************************
 *
 * Body related productions
 *
 **************************************************************************/
body_block
    : statements
    {
        ast::AST_Printer p;

        std::cout << $1 << std::endl;
        if ($1)
            $1->accept(p);

        assert(current_table == root_table);
    }
    |
    ;

statements
    : statement statements { $$ = new ast::Statements($1, $2); }
    | statement { $$ = new ast::Statements($1); }
    ;

statement
    : text { $$ = $1; }
    | control { $$ = $1; }
    | inlined { $$ = $1; }
    ;

text
    : TEXT
    {
        $$ = new ast::Text($1);
        free($1);
    }
    ;

control
    : conditional { $$ = $1; }
    | loop { $$ = $1; }

conditional
    : if end_if
    {
        $$ = new ast::Conditional($1, nullptr, nullptr);
    }
    | if else end_if
    {
        $$ = new ast::Conditional($1, nullptr, $2);
    }
    | if elifs end_if
    {
        $$ = new ast::Conditional($1, $2, nullptr);
    }
    | if elifs else end_if
    {
        $$ = new ast::Conditional($1, $2, $3);
    }
    ;

if
    : if_start expression statements
    {
        /* TODO: check expression->type() == bool */
        /* TODO: maybe convert (e.g. "" => false, "fewaew" => true) */
        $$ = $1;
        $$->set_expression($2);
        $$->set_statements($3);

        /* Return the the surrounding block's symbol table */
        current_table = current_table->parent();
    }
    ;

if_start
    : IF
    {
        /* Create symbol table for the if part */
        current_table = new SymbolTable(current_table);

        $$ = new ast::If(nullptr, current_table);
    }

else
    : else_start statements
    {
        /* TODO: check expression->type() == bool */
        /* TODO: maybe convert (e.g. "" => false, "fewaew" => true) */
        $$ = $1;
        $$->set_statements($2);

        /* Return the the surrounding block's symbol table */
        current_table = current_table->parent();
    }

else_start
    : ELSE
    {
        current_table = new SymbolTable(current_table);
        $$ = new ast::Else(current_table);
    }

elifs
    : elif elifs
    {
        $$ = $1;
        $1->set_next($2);
    }
    | elif
    {
        $$ = $1;
    }

elif
    : elif_start expression statements
    {
        $$ = $1;
        $$->set_expression($2);
        $$->set_statements($3);

        /* Return the the surrounding block's symbol table */
        current_table = current_table->parent();
    }

elif_start
    : ELIF
    {
        current_table = new SymbolTable(current_table);
        $$ = new ast::Elif(nullptr, current_table);
    }

end_if
    : ENDIF { }

 /* TODO: loop and for_each in dire need of cleanup */
loop
    : for_each statements end_for
    {
        $$ = $1;
        $$->set_statements($2);
    }
    | for_each end_for
    {
        /* TODO: remove loop */
        $$ = $1;
    }
    /* TODO: add enumerated for each */
    /* TODO: add for with range */
    ;

for_each
    : FOR IDENTIFIER IN expression
    {
        if ($4->type()->list() == nullptr) {
            vyyerror("Expected a list (got %s)", $4->type()->str().c_str());
            YYERROR;
        }

        /* Create symbol table for the for loop (which will only hold the loop
         * variable) */
        current_table = new SymbolTable(current_table);

        Variable *v = new Variable($2, $4->type()->list()->elem());
        current_table->add(v);

        /* Create symbol table for the statements block */
        current_table = new SymbolTable(current_table);

        $$ = new ast::ForEach(v, $4, current_table->parent(), current_table);

        /* Free the identifier string */
        free($2);
    }

end_for
    : ENDFOR
    {
        /* Go back to symbol table before the for loop */
        current_table = current_table->parent()->parent();
    }

inlined
    : L_INLINE expression R_INLINE
    {
        $$ = new ast::InlinedExpression($2);
    }
    ;

 /* TODO: handle function call */
expression
    : expression AND expression
    {
        try {
            $$ = ast_factory::BoolBinaryFactory<ast::And>::create($1, $3);
        } catch (const DifferentTypesError &e) {
            vyyerror("Invalid type for 'and' operator (%s)",
                e.what());
            YYERROR;
        }
    }
    | expression OR expression
    {
        try {
            $$ = ast_factory::BoolBinaryFactory<ast::Or>::create($1, $3);
        } catch (const DifferentTypesError &e) {
            vyyerror("Invalid type for 'or' operator (%s)",
                e.what());
            YYERROR;
        }
    }
    | expression '+' expression
    {
        try {
            $$ = ast_factory::PlusBinaryFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            vyyerror("%s", e.what());
            YYERROR;
        }
    }
    | expression '-' expression
    {
        try {
            $$ = ast_factory::MinusBinaryFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            vyyerror("%s", e.what());
            YYERROR;
        }
    }
    | expression '*' expression
    {
        try {
            $$ = ast_factory::TimesBinaryFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            vyyerror("%s", e.what());
            YYERROR;
        }
    }
    | single_constant
    {
        $$ = new ast::Constant($1);
    }
    | list
    {
        $$ = $1;
    }
    | IDENTIFIER '.' IDENTIFIER
    {
        try {
            Symbol *s = current_table->lookup($1);

            if (s->get_type()->dot($3) != nullptr) {
                /* TODO */
                vyyerror("TODO IDENTIFIER '.' IDENTIFIER");
                YYERROR;
            }
        } catch (const SymTabNoSuchSymbolError &e) {
            vyyerror("No such symbol: %s\n", e.what());
            YYERROR;
        }

        free($1);
    }
    | IDENTIFIER
    {
        try {
            Symbol *s = current_table->lookup($1);
            $$ = new ast::SymbolRef(s);
        } catch (const SymTabNoSuchSymbolError &e) {
            vyyerror("No such symbol: %s\n", e.what());
            YYERROR;
        }

        free($1);
    }
    | '(' expression ')'
    {
        $$ = $2;
    }

constant
    : single_constant { $$ = $1; }
    | constant_list { $$ = $1; }
    ;

single_constant
    : primitive_constant { $$ = $1; }
    | record_constant { $$ = $1; }

primitive_constant
    : BOOL { $$ = new BoolConstantData($1); }
	| INT { $$ = new IntConstantData($1); }
	| STRING { $$ = new StringConstantData($1); }
	;

list
    : '[' list_values ']'
    {
        $$ = new
            ast::List(TypeFactory::get_list($2->expression->type()->single()));
        $$->set_elements($2);
    }
    | list_type
    {
        $$ = new ast::List($1);
    }
    ;

list_values
    : expression ',' list_values
    {
        if ($1->type()->list()) {
            vyyerror("A list can only hold primitives/records (Got %s)",
                $1->type()->str().c_str());
            YYERROR;
        }

        if ($1->type() != $3->expression->type()) {
            vyyerror("A list can only hold items of same type (Got %s and %s)",
                $1->type()->str().c_str(),
                $3->expression->type()->str().c_str());
            YYERROR;
        }

        $$ = new ast::ExpressionList($1, $3);
    }
    | expression
    {
        if ($1->type()->list()) {
            vyyerror("A list can only hold primitives/records (Got %s)",
                $1->type()->str().c_str());
            YYERROR;
        }

        $$ = new ast::ExpressionList($1);
    }
    ;

%%

void setup_symbol_table()
{
    root_table = new SymbolTable;
    current_table = root_table;

    add_default_functions(*root_table);
}
