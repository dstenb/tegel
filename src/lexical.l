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

%x body
%x control
%x inline
%x pre_inline
%x str
%x text
%x comment

%%

#.*\n                // Comment
[ \t\n]*             // Eat up all whitespace

"arg"                return ARGUMENT;
"record"             return RECORD;
"="                  return '=';
"%%"[ \t]*\n         { BEGIN(body); return SEPARATOR; }


 /* shared rules */
<INITIAL,control,inline>"{"       return '{';
<INITIAL,control,inline>"}"       return '}';
<INITIAL,control,inline>";"       return ';';
<INITIAL,control,inline>"["       return '[';
<INITIAL,control,inline>"]"       return ']';
<INITIAL,control,inline>","       return ',';
<INITIAL,control,inline>"("       return '(';
<INITIAL,control,inline>")"       return ')';
<INITIAL,control,inline>"+"       return '+';
<INITIAL,control,inline>"-"       return '-';
<INITIAL,control,inline>"*"       return '*';
<INITIAL,control,inline>">"       return '>';
<INITIAL,control,inline>"<"       return '<';
<INITIAL,control,inline>">="      return GE;
<INITIAL,control,inline>"<="      return LE;
<INITIAL,control,inline>"=="      return EQ;
<INITIAL,control,inline>"="       return '=';
<INITIAL,control,inline>"!="      return NEQ;
<INITIAL,control,inline>"."       return '.';
<INITIAL,control,inline>and       return AND;
<INITIAL,control,inline>or        return OR;
<INITIAL,control,inline>not       return NOT;
<INITIAL,control,inline>"?"       return '?';
<INITIAL,control,inline>":"       return ':';
<INITIAL,control,inline>for       return FOR;
<INITIAL,control,inline>in        return IN;
<INITIAL,control,inline>endfor    return ENDFOR;
<INITIAL,control,inline>if        return IF;
<INITIAL,control,inline>elif      return ELIF;
<INITIAL,control,inline>else      return ELSE;
<INITIAL,control,inline>endif     return ENDIF;
<INITIAL,control,inline>with      return WITH;
<INITIAL,control,inline>"true"    { yylval.boolean = true; return BOOL; }
<INITIAL,control,inline>"false"   { yylval.boolean = false; return BOOL; }
<INITIAL,control,inline>{INTEGER} { yylval.integer = atoi(yytext); return INT; }

"\""                 { str_caller = INITIAL; string_builder_append("");
                       BEGIN(str); }
{IDENTIFIER}         { yylval.string = strdup(yytext); return IDENTIFIER; }
{IDENTIFIER}"[]"     { yylval.string = strdup(yytext); return LIST; }

 /* string rules */
<str><<EOF>>         { yyerror("syntax error, unmatched '\"'"); yyterminate(); }
<str>"\""            { yylval.string = string_builder; string_builder = NULL;
                       BEGIN(str_caller);
                       return STRING; }
<str>"\\n"           { string_builder_append("\n"); }
<str>"\\t"           { string_builder_append("\t"); }
<str>"\\\""          { string_builder_append("\""); }
<str>"\\\\"          { string_builder_append("\\"); }
<str>"\\".           { vyyerror("unknown escape sequence %c%s", '\\', yytext);
                       yyterminate(); }
<str>{UANYN}         { string_builder_append(yytext); }

<body>[ \t]*"%"      { BEGIN(control); }
<body>(.|\n)         { unput(*yytext); BEGIN(text); }

 /* rules for raw text lines */
<text>"{{"             {
                         if (string_builder) {
                            BEGIN(pre_inline);
                            yylval.string = string_builder;
                            string_builder = NULL;
                            return TEXT;
                         } else {
                            BEGIN(inline);
                            return L_INLINE;
                         }
                       }
<text>"{#"             {
                         BEGIN(comment);
                         if (string_builder) {
                            yylval.string = string_builder;
                            string_builder = NULL;
                            return TEXT;
                         }
                       }
<text>"\\\\\n"         { string_builder_append(yytext);
                         yylval.string = string_builder;
                         string_builder = NULL;
                         yylval.string[strlen(yylval.string) - 3] = '\0';
                         BEGIN(body);
                         return TEXT; }
<text>"\n"             { string_builder_append(yytext);
                         yylval.string = string_builder;
                         string_builder = NULL;
                         BEGIN(body);
                         return TEXT; }
<text>{UANYN}          { string_builder_append(yytext); }
 /* TODO: string_builder should be rewritten in
  * a way that is more suitable for adding single characters */

 /* control section (i.e. "% <control>") */
 /* TODO: add support for continuing the control sequence on the following
  * lines (suggestion: use '\') */
<control>[ \t]*           // Eat up all whitespace
<control>\n               { BEGIN(body); }
<control>"\\"[ \t]*\n     /* Continue the control flow on the next line */

<control>"\""             { str_caller = control; string_builder_append("");
                           BEGIN(str); }

<control,inline>{IDENTIFIER}     { yylval.string = strdup(yytext);
                                   return IDENTIFIER; }
<control,inline>{IDENTIFIER}"[]" { yylval.string = strdup(yytext);
                                   return LIST; }

 /* pre_inline is used as a way to return a TEXT token before L_INLINE */
<pre_inline>(.|\n)        { unput(*yytext); BEGIN(inline); return L_INLINE; }

<inline>[ \t]*            // Eat up all whitespace
<inline>"}}"              { BEGIN(text); return R_INLINE; }
<inline>"\n"              { vyyerror("inlined expressions can't span over "
                            "multiple lines ", '\\', yytext);
                            yyterminate(); }
<inline>"\""              { str_caller = inline; string_builder_append("");
                            BEGIN(str); }

 /* Consider all other characters as errors */
<inline,control,INITIAL>{UANYN} { vyyerror("unexpected character `%s`",
                                  yytext);
                                  yyterminate(); }

 /* comment rules */
<comment><<EOF>>         { yyerror("syntax error, unmatched '#}'");
                           yyterminate(); }
<comment>"#}"            { BEGIN(text); }
<comment>"\\n"           { yyerror("comments can't span over multiple lines");
                           yyterminate(); }
<comment>{UANYN}         // Eat up everything else
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
