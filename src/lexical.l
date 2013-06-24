%{
#include "parser.hpp"

%}

%option noyywrap

IDENTIFIER [[:alpha:]]([[:alnum:]]|_)*

%%

"bool"               { yylval.is_list = false ; return T_BOOL_TYPE; }
"int"                { yylval.is_list = false ; return T_INT_TYPE; }
"string"             { yylval.is_list = false ; return T_STRING_TYPE; }

"bool[]"             { yylval.is_list = true ; return T_BOOL_TYPE; }
"int[]"              { yylval.is_list = true ; return T_INT_TYPE; }
"string[]"           { yylval.is_list = true ; return T_STRING_TYPE; }


%%
