%{
/* Lab2 Attention: You are only allowed to add code in this file and start at Line 26.*/
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos=1;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/

/* @function: getstr
 * @input: a string literal
 * @output: the string value for the input which has all the escape sequences 
 * translated into their meaning.
 */
char *getstr(const char *str)
{
	//optional: implement this function if you need it
	return NULL;
}

static int input(void);

char *stringToken() {
	yyleng = 1;
	int maxlen = 128;
	char *s = (char *)malloc(maxlen);
	int pos = 0;
	char c;
	while((c = (char)input()) != EOF) {
		++yyleng;
		if(c == '\"') {
			break;
		}
		if(c == '\n') {
			break;
		}
		if(c == '\\') {
			c = (char)input();
			++yyleng;
			switch(c) {    // still have other conditions
			case 'n':
				c = '\n';
				break;
			case 't':
				c = '\t';
				break;
			default:
				break;
			}
		}
		if(pos >= maxlen) {
			s = (char *)realloc(s, maxlen * 2);
			maxlen = maxlen * 2;
		}
		s[pos] = (char)c;
		++pos;
	}
	s[pos] = '\0';
	if(pos == 0) {
		free(s);
		return NULL;
	}
	return s;
}

int commentStartCount = 0;

%}
  /* You can add lex definitions here. */

%Start COMMENT
%%
  /* 
  * Below is an example, which you can wipe out
  * and write reguler expressions and actions of your own.
  */ 

"\n" {adjust(); EM_newline(); continue;}
<INITIAL>(" "|"\t")+ {adjust();}

<INITIAL>while {adjust(); return WHILE;}
<INITIAL>for {adjust(); return FOR;}
<INITIAL>to {adjust(); return TO;}
<INITIAL>break {adjust(); return BREAK;}
<INITIAL>let {adjust(); return LET;}
<INITIAL>in {adjust(); return IN;}
<INITIAL>end {adjust(); return END;}
<INITIAL>function {adjust(); return FUNCTION;}
<INITIAL>var {adjust(); return VAR;}
<INITIAL>type {adjust(); return TYPE;}
<INITIAL>array {adjust(); return ARRAY;}
<INITIAL>if {adjust(); return IF;}
<INITIAL>then {adjust(); return THEN;}
<INITIAL>else {adjust(); return ELSE;}
<INITIAL>do {adjust(); return DO;}
<INITIAL>of {adjust(); return OF;}
<INITIAL>nil {adjust(); return NIL;}

<INITIAL>\x22 {yylval.sval=stringToken(); adjust(); return STRING;}
<INITIAL>"/*" {adjust(); ++commentStartCount; BEGIN COMMENT;}

<INITIAL>":=" {adjust(); return ASSIGN;}
<INITIAL>"," {adjust(); return COMMA;}
<INITIAL>":" {adjust(); return COLON;}
<INITIAL>";" {adjust(); return SEMICOLON;}
<INITIAL>"(" {adjust(); return LPAREN;}
<INITIAL>")" {adjust(); return RPAREN;}
<INITIAL>"[" {adjust(); return LBRACK;}
<INITIAL>"]" {adjust(); return RBRACK;}
<INITIAL>"{" {adjust(); return LBRACE;}
<INITIAL>"}" {adjust(); return RBRACE;}
<INITIAL>"." {adjust(); return DOT;}
<INITIAL>"+" {adjust(); return PLUS;}
<INITIAL>"-" {adjust(); return MINUS;}
<INITIAL>"*" {adjust(); return TIMES;}
<INITIAL>"/" {adjust(); return DIVIDE;}
<INITIAL>"=" {adjust(); return EQ;}
<INITIAL>"!="|"<>" {adjust(); return NEQ;}
<INITIAL>"<=" {adjust(); return LE;}
<INITIAL>"<" {adjust(); return LT;}
<INITIAL>">=" {adjust(); return GE;}
<INITIAL>">" {adjust(); return GT;}
<INITIAL>"&" {adjust(); return AND;}
<INITIAL>"|" {adjust(); return OR;}

<INITIAL>[A-Za-z][A-Za-z0-9_]* {adjust(); yylval.sval=String(yytext); return ID;}
<INITIAL>[0-9]+ {adjust(); yylval.ival=atoi(yytext); return INT;}

<INITIAL>. {adjust(); EM_error(charPos, "illegal character");}

<COMMENT>"/*" {adjust(); ++commentStartCount;}
<COMMENT>"*/" {adjust(); --commentStartCount; if(commentStartCount==0) {BEGIN INITIAL;}}
<COMMENT>(" "|"\t")+ {adjust();}
<COMMENT>. {adjust();}

