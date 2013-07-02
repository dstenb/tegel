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
%x str
%x text

%%

#.*\n                // Comment
[ \t\n]*             // Eat up all whitespace

"arg"                return ARGUMENT;
"record"             return RECORD;
"="                  return ASSIGNMENT;
"%%"[ \t]*\n         { BEGIN(body); return SEPARATOR; }


 /* shared rules */
<INITIAL,control,inline>"{"       return L_BRACE;
<INITIAL,control,inline>"}"       return R_BRACE;
<INITIAL,control,inline>";"       return SEMI_COLON;
<INITIAL,control,inline>"["       return L_BRACKET;
<INITIAL,control,inline>"]"       return R_BRACKET;
<INITIAL,control,inline>","       return COMMA;
<INITIAL,control,inline>"("       return L_PAREN;
<INITIAL,control,inline>")"       return R_PAREN;
<INITIAL,control,inline>"+"       return PLUS;
<INITIAL,control,inline>"-"       return MINUS;
<INITIAL,control,inline>"*"       return TIMES;
<INITIAL,control,inline>and       return AND;
<INITIAL,control,inline>or        return OR;
<INITIAL,control,inline>not       return NOT;
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
<str>"\\".           { vyyerror("unknown escape sequence \%s", yytext);
                       yyterminate(); }
<str>{UANYN}         { string_builder_append(yytext); }

<body>[ \t]*"%"      { BEGIN(control); return CONTROL; }
<body>(.|\n)         { unput(*yytext); BEGIN(text); }

 /* rules for raw text lines */
<text>{UANYN}*"\\\\\n" { string_builder_append(yytext);
                         yylval.string = string_builder; string_builder = NULL;
			 yylval.string[strlen(yylval.string) - 3] = '\0';
		         printf("yylval.string = '%s'\n", yylval.string);
			 BEGIN(body); return TEXT;
			}
<text>{UANYN}*"\n"     { string_builder_append(yytext);
                         yylval.string = string_builder; string_builder = NULL;
			 printf("yylval.string = '%s'\n", yylval.string);
			 BEGIN(body); return TEXT; }
 /* TODO: add support for inlining ({{ }}), the above rules will have to be
  * rewritten for single characters, and string_builder should be rewritten in
  * a way that is more suitable for adding single characters */

 /* control section (i.e. "% <control>") */
 /* TODO: add support for continuing the control sequence on the following
  * lines (suggestion: use '\') */
<control>[ \t]*           // Eat up all whitespace
<control>\n               { BEGIN(body); printf("Closing\n"); }
<control>for              return FOR;
<control>in               return IN;
<control>endfor           return ENDFOR;
<control>if               return IF;
<control>elif             return ELIF;
<control>else             return ELSE;
<control>endif            return ENDIF;
<control>"\""             { str_caller = control; string_builder_append("");
                           BEGIN(str); }
<control>{IDENTIFIER}     { yylval.string = strdup(yytext); return IDENTIFIER; }
<control>{IDENTIFIER}"[]" { yylval.string = strdup(yytext); return LIST; }

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