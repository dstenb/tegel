%{
#include "parser.hpp"
#include "type.hpp"
#include <iostream>

%}

%option noyywrap

IDENTIFIER [[:alpha:]]([[:alnum:]]|_)*
INTEGER -?(0|[1-9][0-9]*)

%%

#.*\n                // Comment
[ \t\n]*             // Eat up all whitespace

"arg"                return ARGUMENT;
"="                  return ASSIGNMENT;
"%%"                 return SEPARATOR;
"{"                  return L_BRACE;
"}"                  return R_BRACE;
";"                  return SEMI_COLON;
"["                  return L_BRACKET;
"]"                  return R_BRACKET;

"bool"               { yylval.type = BoolType; return TYPE; }
"int"                { yylval.type = IntType; return TYPE; }
"string"             { yylval.type = StringType; return TYPE; }

"bool[]"             { yylval.type = BoolListType; return TYPE; }
"int[]"              { yylval.type = IntListType; return TYPE; }
"string[]"           { yylval.type = StringListType; return TYPE; }

"true"               { yylval.boolean = true; return BOOL; }
"false"              { yylval.boolean = false; return BOOL; }

{IDENTIFIER}         { yylval.string = strdup(yytext); return IDENTIFIER; }

{INTEGER}            { yylval.integer = atoi(yytext); return INT; }

%%
