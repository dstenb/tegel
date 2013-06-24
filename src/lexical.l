%{
#include "parser.hpp"
#include <iostream>

%}

%option noyywrap

IDENTIFIER [[:alpha:]]([[:alnum:]]|_)*

%%

"arg"                return T_ARGUMENT;
"="                  return T_ASSIGNMENT;
"%%"                 return T_SEPARATOR;

"bool"               { yylval.is_list = false ; return T_BOOL_TYPE; }
"int"                { yylval.is_list = false ; return T_INT_TYPE; }
"string"             { yylval.is_list = false ; return T_STRING_TYPE; }

"bool[]"             { yylval.is_list = true ; return T_BOOL_TYPE; }
"int[]"              { yylval.is_list = true ; return T_INT_TYPE; }
"string[]"           { yylval.is_list = true ; return T_STRING_TYPE; }

{IDENTIFIER}         { std::cerr << "Identifier(" << yytext << ")\n";
			return T_IDENTIFIER; } // TODO yylval

-?(0|[1-9][0-9]*)    { std::cerr << "Integer(" << atoi(yytext) << ")\n";
			return T_INT; } // TODO yylval
true                 { std::cerr << "Bool(true)\n"; return T_BOOL; } // TODO yylval
false                { std::cerr << "Bool(false)\n"; return T_BOOL; } // TODO yylval

%%
