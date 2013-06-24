%{
#include "parser.hpp"
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

"bool"               { yylval.is_list = false ; return T_BOOL_TYPE; }
"int"                { yylval.is_list = false ; return T_INT_TYPE; }
"string"             { yylval.is_list = false ; return T_STRING_TYPE; }

"bool[]"             { yylval.is_list = true ; return T_BOOL_TYPE; }
"int[]"              { yylval.is_list = true ; return T_INT_TYPE; }
"string[]"           { yylval.is_list = true ; return T_STRING_TYPE; }

"true"               { std::cerr << "Bool(true)\n";
			yylval.boolean = true; return T_BOOL; }
"false"              { std::cerr << "Bool(false)\n";
			yylval.boolean = false; return T_BOOL; }

{IDENTIFIER}         { yylval.string = strdup(yytext); return T_IDENTIFIER; }

{INTEGER}            { std::cerr << "Integer(" << atoi(yytext) << ")\n";
			yylval.integer = atoi(yytext);
			return T_INT; } // TODO yylval
%%
