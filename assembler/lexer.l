/* this will include some C code directly, so we can pull in the
 * header files we want */
%{
#include "parser.hpp"
#include "helpers.hpp"
#include <string>
int line_num = 0;
%}

/* this tells us that we're going to output our lexer implementation
 * to `lexer.c` and the associated header to `lexer.h`.
 */
%option outfile="lexer.cpp" header-file="lexer.hpp"

/* the int yywrap(void) is used internally by the lexer for the case
 *  where you want to parse multiple files in sequence with the same
 * parser state. Usually, you'd supply your own implementation that
 * would set up the next input stream and let the lexer know whether
 * to keep going. In our case, we disable it; this is effectively
 * the same as a dummy implementation like #define yywrap() 0.
 */
%option noyywrap
/* these define a few literals to use later on */
/* LPAR  "("
RPAR  ")"
PLUS  "+"
SEMI  ";" */
COMMA ","

/* and these define some regular expressions we'll use. We could
 * also use these inline down below. 
HEX   0[Xx][0-9A-Fa-f]+ */
COM   "#"[^\n\r]*
WS    [ \r\t]*
GLOBAL \.global
SYMBOL [a-zA-Z][a-zA-Z0-9_]*

/* In the below code, the TOKEN_blah types and the yylval
 * structure are used; these are defined in parser.y */

%%

{WS}        { /* skip whitspace */ }
{COM}       { /* skip comments */ }
{SYMBOL} {
	yylval.symbol = copy_str(yytext);
	return TOKEN_SYMBOL;
}
{GLOBAL} {
    yylval.symbol = copy_str(yytext);
		return TOKEN_GLOBAL;
}
{COMMA} { return TOKEN_COMMA; }
\n             { ++line_num;}
. {}
%%

/* In the event of some kind of error, this will get called. */
int
yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}
