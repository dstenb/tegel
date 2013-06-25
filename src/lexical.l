%{
#include <iostream>

#include "parser.hpp"
#include "type.hpp"

char *string_builder = NULL;

void string_builder_append(const char *s);
char *string_builder_take(void);

%}

%option noyywrap

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

"\""                 { BEGIN(str); }

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

<str><<EOF>>         { std::cerr << yylineno << "unclosed\n";  yyterminate(); }
<str>"\""            { yylval.string = string_builder_take(); return STRING; }
<str>"\\n"           { string_builder_append("\n"); }
<str>"\\t"           { string_builder_append("\t"); }
<str>"\\\""          { string_builder_append("\""); }
<str>"\\\\"          { string_builder_append("\\"); }
<str>"\\".           { std::cerr << "unknown escape sequence \\" << yytext <<
"\n"; }

<str>{UANYN}         { string_builder_append(yytext); }

<str>.               { std::cerr << "okänd: '" << yytext << "'" << std::endl; }

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

char *string_builder_take(void)
{
	char *p = string_builder;
	string_builder = NULL;
	return p;
}
