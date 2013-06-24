%{
#include "parser.hpp"
#include "type.hpp"
#include <iostream>

%}

%option noyywrap

IDENTIFIER [[:alpha:]]([[:alnum:]]|_)*
INTEGER -?(0|[1-9][0-9]*)

%%

"arg"                return T_ARGUMENT;
"="                  return T_ASSIGNMENT;
"%%"                 return T_SEPARATOR;
"{"                  return T_L_BRACE;
"}"                  return T_R_BRACE;

"bool"               { yylval.type = BoolType; return T_TYPE; }
"int"                { yylval.type = IntType; return T_TYPE; }
"string"             { yylval.type = StringType; return T_TYPE; }

"bool[]"             { yylval.type = BoolListType; return T_TYPE; }
"int[]"              { yylval.type = IntListType; return T_TYPE; }
"string[]"           { yylval.type = StringListType; return T_TYPE; }

"true"               { std::cerr << "Bool(true)\n";
			yylval.boolean = true; return T_BOOL; }
"false"              { std::cerr << "Bool(false)\n";
			yylval.boolean = false; return T_BOOL; }

{IDENTIFIER}         { yylval.string = strdup(yytext); return T_IDENTIFIER; }

{INTEGER}            { std::cerr << "Integer(" << atoi(yytext) << ")\n";
			yylval.integer = atoi(yytext);
			return T_INT; } // TODO yylval
%%
