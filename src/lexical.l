%{
#include <iostream>
#include <cstdarg>

#include "parser.hpp"

char *string_builder = NULL;

void string_builder_append(const char *s);

void yyerror(const char *);
void vyyerror(const char *, ...);

int str_caller; // calling state for str
%}

%option noyywrap
%option yylineno

IDENTIFIER [[:alpha:]]([[:alnum:]]|_)*
INTEGER -?(0|[1-9][0-9]*)

ASC     [\x00-\x7f]
ASCN    [\x00-\t\v-\x7f]
U       [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]

UANY    {ASC}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}
UANYN   {ASCN}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}
UONLY   {U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}

%x str

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
","                  return COMMA;

"\""                 { str_caller = INITIAL; BEGIN(str); }

"true"               { yylval.boolean = true; return BOOL; }
"false"              { yylval.boolean = false; return BOOL; }

{IDENTIFIER}         { yylval.string = strdup(yytext); return IDENTIFIER; }
{IDENTIFIER}"[]"     { yylval.string = strdup(yytext); return LIST; }

{INTEGER}            { yylval.integer = atoi(yytext); return INT; }

<str><<EOF>>         { yyerror("syntax error, unmatched '\"'"); yyterminate(); }
<str>"\""            { yylval.string = string_builder; string_builder = NULL;
                       BEGIN(str_caller);
		       return STRING; }
<str>"\\n"           { string_builder_append("\n"); }
<str>"\\t"           { string_builder_append("\t"); }
<str>"\\\""          { string_builder_append("\""); }
<str>"\\\\"          { string_builder_append("\\"); }
<str>"\\".           { vyyerror("unknown escape sequence \%s", yytext);
                       yyterminate(); }
<str>{UANYN}         { string_builder_append(yytext); }

%%

void string_builder_append(const char *s)
{
	if (!string_builder) {
		string_builder = strdup(s);
		// TODO: handle failed alloc
		return;
	} else {
		size_t sb = strlen(string_builder);
		size_t sn = strlen(s);

		string_builder = (char *)realloc(string_builder, sb + sn + 1);

		// TODO: handle failed realloc
		if (string_builder != NULL)
			memcpy(string_builder + sb, s, sn);
		string_builder[sb + sn] = '\0';
	}
}

void yyerror(const char *s)
{
	fprintf(stderr, "Error on line %i: %s\n", yylineno, s);
}

void vyyerror(const char *fmt, ...)
{
	va_list val;

	fprintf(stderr, "Error on line %i: ", yylineno);

	va_start(val, fmt);
	vfprintf(stderr, fmt, val);
	va_end(val);
	fprintf(stderr, "\n");
}
