%{
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "ast.hpp"
#include "ast_factories.hpp"
#include "ast_printer.hpp"
#include "common.hpp"
#include "data.hpp"
#include "symbol.hpp"

using namespace constant;
using namespace symbol;

/* constant_list is used by the constant_list grammar rule to hold the list
  elements. This is used instead of a AST approach and is ok since only one
  list will be handled at a time (since a list can only contain primitives and
  records) same reasoning for constant_record */
std::vector<SingleConstantData *> constant_list;
std::vector<PrimitiveConstantData *> constant_record;

/* same reasoning as above */
std::vector<Param *> param_list;

RecordType::field_vector record_members;

std::map<string, ast::Expression *> kw_map;

#define scanner context->scanner
%}

%locations
%error-verbose
%pure-parser
%lex-param { void *scanner }
%parse-param { ParseContext *context }

%code requires {
    #include "ast.hpp"
    #include "ast_factories.hpp"
    #include "constant.hpp"
    #include "symbol.hpp"
    #include "type.hpp"
    #include "data.hpp"

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
    ast::Record *record;
    ast::ExpressionList *expression_list;
    ast::VariableList *variable_list;
    ast::VariableStatement *variable_stmt;
    ast::FuncArgList *funcarg_list;

    ast::LambdaExpression *lambda;
}

%{
int yylex(YYSTYPE *, YYLTYPE *, void *);
void yyerror(YYLTYPE *, ParseContext *, const char *);
void yyverror(YYLTYPE *, ParseContext *, const char *, ...);
void yywarning(YYLTYPE *, ParseContext *, const char *);
void yyvwarning(YYLTYPE *, ParseContext *, const char *, ...);

%}

%token END 0 "end of file"
%token ARGUMENT "argument"
%token RECORD "record"
%token SEPARATOR "%%"
%token<string> IDENTIFIER "identifier"
%token FOR "for" IN "in" ENDFOR "endfor"
%token IF "if" ELIF "elif" ELSE "else" ENDIF "endif"
%token AND "and" OR "or" NOT "not"
%token WITH "with"
%token CREATE "create"
%token L_INLINE "{{" R_INLINE "}}"
%token LE "<=" EQ "==" NEQ "!=" GE ">="
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

%type<funcarg_list> function_args

%type<argument> arg

%type<param> param

%type<type> type
%type<stype> single_type
%type<ltype> list_type

%type<statements> statements

%type<statement> statement text conditional control inlined
%type<statement> create
%type<scope> loop for_each for_each_enum
%type<if_node> if if_start
%type<elif_node> elif_start elifs elif
%type<else_node> else_start else

%type<variable_list> with variable_list variable_decl_list
%type<variable_stmt> variable_decl_assign variable_assign variable_decl

%type<expression> expression condition function_call
%type<list> list
%type<record> record
%type<expression_list> expression_list list_values

%type<lambda> lambda lambda_start

%right '?' ':'
%left OR
%left AND
%left EQ NEQ
%left NOT
%left '<' '>' LE GE
%left '+' '-'
%left '*'
%left '.' '#'

%start file

%%

file
    : header_block SEPARATOR body_block
    {
        /*context->data->root_table->print(std::cerr);*/
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
        try {
            context->data->root_table->add($1);
            context->data->arguments.push_back($1);
        } catch(const SymTabAlreadyDefinedError &e) {
            stringstream sstr;
            context->data->root_table->lookup($1->get_name())->print(sstr);
            yyverror(&@1, context, "'%s' is already defined (as %s)\n",
                $1->get_name().c_str(), sstr.str().c_str());
            YYERROR;
        }
    }
    | record_def
    ;

arg
    : ARGUMENT type IDENTIFIER '{' header_item_params '}'
    {
        try {
            $$ = Argument::create($3, $2);
        } catch (const SymbolNameError &e) {
            yyverror(&@3, context, e.what());
            YYERROR;
        }

        for (Param *p : param_list) {
            try {
                Param *op = $$->replace(p);
                if (op)
                    delete op;
            } catch (const ParamException &e) {
                yyerror(&@1, context, e.what());
                YYERROR;
            }
        }
        param_list.clear();
        free($3); // free identifier string
    }

record_def
    : RECORD IDENTIFIER '{' record_def_members '}'
    {
        try {
            TypeFactory::add_record($2, record_members);
        } catch (const TypeAlreadyDefined &e) {
	    yyerror(&@2, context, e.what());
            YYERROR;
        }
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
            yyverror(&@2, context,
                "a record can only hold primitive types (got %s)", $1);
            YYERROR;
        }

        auto it = find_if(record_members.begin(), record_members.end(),
            [&] (const RecordField &r) { return r.name == $2; });

        if (it != record_members.end()) {
            yyverror(&@2, context, "Multiple definitions of field '%s'", $2);
            YYERROR;
        }

        record_members.push_back({ $2, p});

        free($2);
    }
    | list_type IDENTIFIER ';'
    {
    	yyverror(&@2, context,
            "a record can only hold primitive types (got %s)", $1);
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
            yyverror(&@1, context, "unknown type '%s'", $1);
            YYERROR;
        }

        const RecordType *p = t->record();

        if (p == nullptr) {
            yyverror(&@1, context,
	        "expected a record (got '%s')", t->str().c_str());
        }

        $$ = new RecordConstantData(p);

        try {
            $$->set(constant_record);
        } catch (const UnmatchingFieldSignature &e) {
            yyerror(&@1, context, e.what());
            YYERROR;
        }

        constant_record.clear();

        free($1);
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
            yyverror(&@1, context, "unknown type '%s'", $1);
            YYERROR;
        }

        free($1);
    }
    ;

list_type
    : LIST
    {
        const Type *t = TypeFactory::get($1);

        $$ = t ? t->list() : nullptr;

        if ($$ == nullptr) {
            yyverror(&@1, context, "unknown type '%s'", $1);
            YYERROR;
        }

        free($1);
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
            yyerror(&@1, context, e.what());
            YYERROR;
        } catch (const DifferentTypesError &e) {
            yyverror(&@1, context,
	        "a list can only hold items of one type (%s)", e.what());
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
        context->data->body = $1;
        assert(context->data->current_table == context->data->root_table);
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
    | with { $$ = $1; }
    | create { $$ = $1; }

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
    : if_start condition statements
    {
        $$ = $1;
        $$->set_condition($2);
        $$->set_statements($3);

        /* Return the the surrounding block's symbol table */
        context->data->current_table = context->data->current_table->parent();
    }
    ;

if_start
    : IF
    {
        /* Create symbol table for the if part */
        context->data->current_table = new SymbolTable(context->data->current_table);

        $$ = new ast::If(nullptr, context->data->current_table);
    }

else
    : else_start statements
    {
        $$ = $1;
        $$->set_statements($2);

        /* Return the the surrounding block's symbol table */
        context->data->current_table = context->data->current_table->parent();
    }

else_start
    : ELSE
    {
        context->data->current_table = new SymbolTable(context->data->current_table);
        $$ = new ast::Else(context->data->current_table);
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
    : elif_start condition statements
    {
        $$ = $1;
        $$->set_condition($2);
        $$->set_statements($3);

        /* Return the the surrounding block's symbol table */
        context->data->current_table = context->data->current_table->parent();
    }

elif_start
    : ELIF
    {
        context->data->current_table = new SymbolTable(context->data->current_table);
        $$ = new ast::Elif(nullptr, context->data->current_table);
    }

end_if
    : ENDIF { }

loop
    : for_each statements end_for
    {
        $$ = $1;
        $$->set_statements($2);
    }
    | for_each end_for
    {
        /* This will be optimized away in the backend */
        $$ = $1;
    }
    | for_each_enum statements end_for
    {
        $$ = $1;
        $$->set_statements($2);
    }
    | for_each_enum end_for
    {
        /* This will be optimized away in the backend */
        $$ = $1;
    }
    ;

for_each
    : FOR IDENTIFIER IN expression
    {
        Variable *v;

        if ($4->type()->list() == nullptr) {
            yyverror(&@4, context,
                "expected a list (got %s)", $4->type()->str().c_str());
            YYERROR;
        }

        /* Create symbol table for the for loop (which will only hold the loop
         * variable) */
        context->data->current_table = new SymbolTable(context->data->current_table);

        try {
            v = Variable::create($2, $4->type()->list()->elem());
        } catch (const SymbolNameError &e) {
            yyverror(&@2, context, e.what());
            YYERROR;
        }
        context->data->current_table->add(v);

        /* Create symbol table for the statements block */
        context->data->current_table = new SymbolTable(context->data->current_table);

        $$ = new ast::ForEach(v, $4, context->data->current_table->parent(),
            context->data->current_table);

        /* Free the identifier string */
        free($2);
    }

for_each_enum
    : FOR IDENTIFIER ',' IDENTIFIER IN expression
    {
        Variable *i, *v;

        if ($6->type()->list() == nullptr) {
            yyverror(&@6, context,
	    	"expected a list (got %s)", $6->type()->str().c_str());
            YYERROR;
        }

        /* Create symbol table for the for loop (which will only hold the loop
         * variable) */
        context->data->current_table = new SymbolTable(context->data->current_table);

        try {
            i = Variable::create($2, TypeFactory::get("int"));
        } catch (const SymbolNameError &e) {
            yyverror(&@2, context, e.what());
            YYERROR;
        }
        context->data->current_table->add(i);

        try {
            v = Variable::create($4, $6->type()->list()->elem());
        } catch (const SymbolNameError &e) {
            yyverror(&@4, context, e.what());
            YYERROR;
        }
        context->data->current_table->add(v);

        /* Create symbol table for the statements block */
        context->data->current_table = new SymbolTable(context->data->current_table);

        $$ = new ast::ForEachEnum(i, v, $6, context->data->current_table->parent(),
            context->data->current_table);

        /* Free the identifier string */
        free($2);
    }

end_for
    : ENDFOR
    {
        /* Go back to symbol table before the for loop */
        context->data->current_table = context->data->current_table->parent()->parent();
    }

with
    : WITH variable_list { $$ = $2; }

create
    : CREATE '(' expression ',' STRING ',' BOOL create_keywords ')'
    {
        /* TODO: use absolute paths (realpath()) */

        if (context->is_tgp()) {
            if ($3->type() != TypeFactory::get("string")) {
                yyverror(&@3, context,
                    "wrong type for first argument to create() (got %s, "
                    "expected string", $3->type()->str().c_str());
                YYERROR;
            }

            auto data = context->get_parsed_file($5);

            if (!data) {
                FILE *fp;

                if (!(fp = fopen($5, "r"))) {
                    yyverror(&@5, context, "couldn't open %s (%s)",
                        $5, strerror(errno));
                    YYERROR;
                }

                ParseContext *new_context = new ParseContext($5, fp, false);

                if (yyparse(new_context) != 0)
                    YYERROR;

                context->set_parsed_file($5, new_context->data);
                data = new_context->data;
            }

            for (auto it = kw_map.begin(); it != kw_map.end(); ++it) {
                auto fit = find_if(data->arguments.begin(),
                    data->arguments.end(), [&] (Argument *a) {
                        return a->get_name() == it->first; });

                if (fit == data->arguments.end()) {
                    yyverror(&@1, context,
                        "invalid keyword for '%s': '%s'", $5,
                        it->first.c_str());
                    YYERROR;
                } else if (it->second->type() != (*fit)->get_type()) {
                    yyverror(&@1, context,
                        "wrong type for keyword '%s' (got %s, expected %s)",
                        it->first.c_str(), it->second->type()->str().c_str(),
                        (*fit)->get_type()->str().c_str());
                    YYERROR;
                }
            }

            $$ = new ast::Create($3, $5, $7, kw_map);
        } else {
            yyerror(&@1, context, "create is only allowed in .tgp files (use "
                "tegel-tgp instead");
            YYERROR;
        }

        kw_map.clear();
        free($5);
    }

create_keywords
    : ',' keyword_list { /* empty */  }
    | /* empty */

variable_list
    : variable_decl_assign ',' variable_list
    {
        $$ = new ast::VariableList($1, $3);
    }
    | variable_decl_assign
    {
        $$ = new ast::VariableList($1, nullptr);
    }
    | variable_assign ',' variable_list
    {
        $$ = new ast::VariableList($1, $3);
    }
    | variable_assign
    {
        $$ = new ast::VariableList($1, nullptr);
    }

variable_decl_assign
    : type IDENTIFIER '=' expression
    {
        try {
            auto v = Variable::create($2, $1);

            context->data->current_table->add(v);

            if ($1 != $4->type()) {
                yyverror(&@4, context,
			"invalid type for assignment to %s (got %s, "
                    "expected)%s", $2, $1->str().c_str(),
                    $4->type()->str().c_str());
                YYERROR;
            }
            $$ = new ast::VariableDeclaration(v, $4);
        } catch (const SymbolNameError &e) {
            yyverror(&@2, context, e.what());
            YYERROR;
        } catch(const SymTabAlreadyDefinedError &e) {
            stringstream sstr;
            context->data->current_table->lookup($2)->print(sstr);
            yyverror(&@2, context, "'%s' is already defined (as %s)\n",
                $2, sstr.str().c_str());
            YYERROR;
        }
        free($2);
    }

variable_assign
    : IDENTIFIER '=' expression
    {
        try {
            Symbol *s = context->data->current_table->lookup($1);

            if (s->read_only()) {
                yyverror(&@1, context, "%s is not assignable\n", $1);
                YYERROR;
            }

            if (s->get_type() != $3->type()) {
                yyverror(&@3, context,
			"invalid type for assignment to %s (got %s, "
                    "expected %s)", $1, s->get_type()->str().c_str(),
                    $3->type()->str().c_str());
                YYERROR;
            }
            $$ = new ast::VariableAssignment(s->variable(), $3); 
        } catch (const SymTabNoSuchSymbolError &e) {
            yyverror(&@1, context, "no such variable: %s\n", e.what());
            YYERROR;
        }
        free($1);
    }

inlined
    : L_INLINE expression R_INLINE
    {
        $$ = new ast::InlinedExpression(ast_factory::StringFactory::create($2));
    }
    ;

expression
    : expression '?' expression ':' expression
    {
        try {
            $$ = ast_factory::TernaryIfFactory::create($1, $3, $5);
        } catch (const InvalidTypeError &e) {
            yyverror(&@2, context, e.what());
            YYERROR;
        }
    }
    | expression AND expression
    {
        try {
            $$ = ast_factory::BoolBinaryFactory<ast::And>::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression OR expression
    {
        try {
            $$ = ast_factory::BoolBinaryFactory<ast::Or>::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | NOT expression
    {
        try {
            $$ = new ast::Not(ast_factory::BoolUnaryFactory::create($2));
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression '<' expression
    {
        try {
            $$ = ast_factory::LessThanFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression '>' expression
    {
        try {
            $$ = ast_factory::GreaterThanFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression LE expression
    {
        try {
            $$ = ast_factory::LessThanOrEqualFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression GE expression
    {
        try {
            $$ = ast_factory::GreaterThanOrEqualFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression EQ expression
    {
        try {
            $$ = ast_factory::EqualsFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression NEQ expression
    {
        try {
            $$ = new ast::Not(ast_factory::EqualsFactory::create($1, $3));
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression '+' expression
    {
        try {
            $$ = ast_factory::PlusBinaryFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression '-' expression
    {
        try {
            $$ = ast_factory::MinusBinaryFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | expression '*' expression
    {
        try {
            $$ = ast_factory::TimesBinaryFactory::create($1, $3);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | primitive_constant
    {
        $$ = new ast::Constant($1);
    }
    | record
    {
        $$ = $1;
    }
    | list
    {
        $$ = $1;
    }
    | expression '.' IDENTIFIER
    {
        if ($1->type()->record()) {
            auto t = $1->type()->dot($3);

            if (t == nullptr) {
                yyverror(&@3, context, "'%s' has no field named '%s'",
                    $1->type()->str().c_str(), $3);
                YYERROR;
            }
            $$ = new ast::FieldRef($1, $3);
        } else {
            yyverror(&@1, context,
	    	"can't apply '.' operator on expression of type '%s'",
                $1->type()->str().c_str());
            YYERROR;
        }

        free($3);
    }
    | expression '.' IDENTIFIER '(' expression_list ')'
    {
        try {
            vector<const Type *> arg_types;
            for (auto p = $5; p != nullptr; p = p->next)
                arg_types.push_back(p->expression->type());
            TypeMethod m = $1->type()->lookup($3, arg_types);
            $$ = new ast::MethodCall($1, m, $5);
        } catch (const NoSuchMethodError &e) {
            yyerror(&@3, context, e.what());
            YYERROR;
        } catch (const WrongNumberOfArgumentsError &e) {
            yyerror(&@3, context, e.what());
            YYERROR;
        } catch (const WrongArgumentSignatureError &e) {
            yyerror(&@3, context, e.what());
            YYERROR;
        }

        free($3);
    }
    | IDENTIFIER
    {
        try {
            Symbol *s = context->data->current_table->lookup($1);
            $$ = new ast::SymbolRef(s);
        } catch (const SymTabNoSuchSymbolError &e) {
            yyverror(&@1, context, "no such symbol: %s\n", e.what());
            YYERROR;
        }

        free($1);
    }
    | '-' expression
    {
        try {
            $$ = ast_factory::MinusUnaryFactory::create($2);
        } catch (const InvalidTypeError &e) {
            yyerror(&@2, context, e.what());
            YYERROR;
        }
    }
    | '#' expression
    {
        try {
            $$ = ast_factory::LengthUnaryFactory::create($2);
        } catch (const InvalidTypeError &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }
    }
    | '(' expression ')'
    {
        $$ = $2;
    }
    | function_call
    {
        $$ = $1;
    }

condition
    : expression
    {
        try {
            $$ = ast_factory::BoolUnaryFactory::create($1);
        } catch (const InvalidTypeError &e) {
            yyerror(&@1, context, e.what());
            YYERROR;
        }
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

record
    : IDENTIFIER '{' expression_list '}'
    {
        const Type *t = TypeFactory::get($1);

        if (t == nullptr) {
            yyverror(&@1, context, "unknown type '%s'", $1);
            YYERROR;
        }

        const RecordType *p = t->record();

        if (p == nullptr) {
            yyverror(&@1, context,
	        "expected a record (got '%s')", t->str().c_str());
            YYERROR;
        }

        $$ = new ast::Record(p);
        try {
            $$->set_fields($3);
        } catch (const UnmatchingFieldSignature &e) {
            yyverror(&@1, context, e.what());
            YYERROR;
        }

        free($1);
    }

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
            yyverror(&@3, context,
	    	"a list can only hold primitives/records (Got %s)",
                $1->type()->str().c_str());
            YYERROR;
        }

        if ($1->type() != $3->expression->type()) {
            yyverror(&@1, context,
	    	"a list can only hold items of one type (Got %s and %s)",
                $1->type()->str().c_str(),
                $3->expression->type()->str().c_str());
            YYERROR;
        }

        $$ = new ast::ExpressionList($1, $3);
    }
    | expression
    {
        if ($1->type()->list()) {
            yyverror(&@1, context,
	    	"a list can only hold primitives/records (Got %s)",
                $1->type()->str().c_str());
            YYERROR;
        }

        $$ = new ast::ExpressionList($1);
    }
    ;

expression_list
    : expression ',' expression_list
    {
        $$ = new ast::ExpressionList($1, $3);
    }
    | expression
    {
        $$ = new ast::ExpressionList($1);
    }
    |
    {
        $$ = nullptr;
    }
    ;

variable_decl_list
    : variable_decl ',' variable_decl_list
    {
        $$ = new ast::VariableList($1, $3);
    }
    | variable_decl
    {
        $$ = new ast::VariableList($1, nullptr);
    }
    ;

lambda_start
    : '^'
    {
        context->data->current_table = new SymbolTable(context->data->current_table);
        $$ = new ast::LambdaExpression(nullptr, nullptr, context->data->current_table);
    }
    ;

lambda
    : lambda_start variable_decl_list ':' expression
    {
        $$ = $1;
        $$->variables = $2;
        $$->expression = $4;
        context->data->current_table = context->data->current_table->parent();
    }

function_args
    : expression ',' function_args
    {
        ast::FuncArgExpression *arg = new ast::FuncArgExpression($1);
        $$ = new ast::FuncArgList(arg, $3);
    }
    | expression
    {
        ast::FuncArgExpression *arg = new ast::FuncArgExpression($1);
        $$ = new ast::FuncArgList(arg);
    }
    | lambda ',' function_args
    {
        ast::FuncArgLambda *arg = new ast::FuncArgLambda($1);
        $$ = new ast::FuncArgList(arg, $3);
    }
    | lambda
    {
        ast::FuncArgLambda *arg = new ast::FuncArgLambda($1);
        $$ = new ast::FuncArgList(arg);
    }
    |
    {
        $$ = nullptr;
    }
    ;

function_call
    : IDENTIFIER '(' function_args ')'
    {
        try {
            $$ = ast_factory::FunctionCallFactory::create($1, $3);
        } catch (const ast_factory::NoSuchFunctionError &e) {
            yyerror(&@3, context, e.what());
            YYERROR;
        } catch (const ast_factory::WrongLambdaSignatureError &e) {
            yyerror(&@3, context, e.what());
            YYERROR;
        } catch (const ast_factory::WrongFunctionSignatureError &e) {
            yyerror(&@3, context, e.what());
            YYERROR;
        }
    }
    ;

variable_decl
    : type IDENTIFIER
    {
        try {
            auto v = Variable::create($2, $1);

            context->data->current_table->add(v);

            $$ = new ast::VariableDeclaration(v);
        } catch (const SymbolNameError &e) {
            yyverror(&@2, context, e.what());
            YYERROR;
        } catch(const SymTabAlreadyDefinedError &e) {
            stringstream sstr;
            context->data->current_table->lookup($2)->print(sstr);
            yyverror(&@2, context, "'%s' is already defined (as %s)\n",
                $2, sstr.str().c_str());
            YYERROR;
        }
        free($2);
    }
    ;

keyword_list
    : IDENTIFIER '=' expression ',' keyword_list
    {
        auto it = kw_map.find($1);
        if (it != kw_map.end()) {
            yyvwarning(&@1, context,
                "multiple declarations of argument '%s'", $1);
            auto e = it->second;
            it->second = $3;
            delete e;
        } else {
            kw_map[$1] = $3;
        }
    }
    | IDENTIFIER '=' expression
    {
        auto it = kw_map.find($1);
        if (it != kw_map.end()) {
            yyvwarning(&@1, context,
                "multiple declarations of argument '%s'", $1);
            auto e = it->second;
            it->second = $3;
            delete e;
        } else {
            kw_map[$1] = $3;
        }
    }
    | /* empty */
    ;

%%
