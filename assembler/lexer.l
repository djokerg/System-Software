/* this will include some C code directly, so we can pull in the
 * header files we want */
%{
#include "parser.hpp"
#include "helpers.hpp"
#include <string.h>
int line_num = 1;
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
LSQB  "["
RSQB  "]"
PLUS  "+"
COMMA ","

/* and these define some regular expressions we'll use. We could
 * also use these inline down below. 
HEX   0[Xx][0-9A-Fa-f]+ */
COM   #.*
WS    [ \r\t]*
GLOBAL \.global
EXTERN \.extern
SECTION \.section
WORD \.word
SKIP \.skip
ASCII \.ascii
EQU \.equ
END \.end
STRING \"[^"]+\"
LITERAL_DECIMAL -?[0-9]+
LITERAL_HEXADECIMAL 0x[0-9A-F]+
LITERAL 0x[0-9A-F]+|-?[0-9]+
SYMBOL [a-zA-Z][a-zA-Z0-9_]*
LABEL [a-zA-Z][a-zA-Z0-9_]*:
GP_REGISTER %(r[0-9]+|sp|pc)
CS_REGISTER %(status|handler|cause)


/* In the below code, the TOKEN_blah types and the yylval
 * structure are used; these are defined in parser.y */

%%

{WS}        { }
{COM}       { return TOKEN_COMM; }
\$ {
	return TOKEN_IMM;
}
{LSQB} {
	return TOKEN_LSQB;
}
{RSQB} {
	return TOKEN_RSQB;
}
{PLUS} {
	return TOKEN_PLUS;
}
{GLOBAL} {
    yylval.symbol = copy_str(yytext);
		return TOKEN_GLOBAL;
}
{EXTERN} {
     yylval.symbol = copy_str(yytext);
		return TOKEN_EXTERN;
}
{SECTION} {
     yylval.symbol = copy_str(yytext);
		return TOKEN_SECTION;
}
{WORD} {
     yylval.symbol = copy_str(yytext);
		return TOKEN_WORD;
}
{SKIP} {
     yylval.symbol = copy_str(yytext);
		return TOKEN_SKIP;
}
{ASCII} {
     yylval.symbol = copy_str(yytext);
		return TOKEN_ASCII;
}
{EQU} {
     yylval.symbol = copy_str(yytext);
		return TOKEN_EQU;
}
{END} {
     yylval.symbol = copy_str(yytext);
		return TOKEN_END;
}
{LITERAL_DECIMAL} {
	yylval.num = atoi(yytext);
	return TOKEN_LITERAL;
}
{LITERAL_HEXADECIMAL} {
	yylval.num = strtol(yytext, NULL, 16);
	return TOKEN_LITERAL;
}
{STRING} {
     yylval.symbol = copy_str(yytext);
	return TOKEN_STRING;
}
{LABEL} {
     yylval.symbol = copy_str(yytext);
	return TOKEN_LABEL;
}
halt {
	yylval.symbol = copy_str(yytext);
	return TOKEN_HALT;
}
int {
	yylval.symbol = copy_str(yytext);
	return TOKEN_INT;
}
iret {
	yylval.symbol = copy_str(yytext);
	return TOKEN_IRET;
}
call {
     yylval.symbol = copy_str(yytext);
	return TOKEN_CALL;
}
ret {
     yylval.symbol = copy_str(yytext);
	return TOKEN_RET;
}
jmp {
     yylval.symbol = copy_str(yytext);
	return TOKEN_JMP;
}
beq {
     yylval.symbol = copy_str(yytext);
	return TOKEN_BEQ;
}
bne {
     yylval.symbol = copy_str(yytext);
	return TOKEN_BNE;
}
bgt {
     yylval.symbol = copy_str(yytext);
	return TOKEN_BGT;
}
push {
     yylval.symbol = copy_str(yytext);
	return TOKEN_PUSH;
}
pop {
     yylval.symbol = copy_str(yytext);
	return TOKEN_POP;
}
xchg {
     yylval.symbol = copy_str(yytext);
	return TOKEN_XCHG;
}
add {
     yylval.symbol = copy_str(yytext);
	return TOKEN_ADD;
}
sub {
     yylval.symbol = copy_str(yytext);
	return TOKEN_SUB;
}
mul {
     yylval.symbol = copy_str(yytext);
	return TOKEN_MUL;
}
div {
     yylval.symbol = copy_str(yytext);
	return TOKEN_DIV;
}
not {
     yylval.symbol = copy_str(yytext);
	return TOKEN_NOT;
}
and {
     yylval.symbol = copy_str(yytext);
	return TOKEN_AND;
}
or {
     yylval.symbol = copy_str(yytext);
	return TOKEN_OR;
}
xor {
     yylval.symbol = copy_str(yytext);
	return TOKEN_XOR;
}
shl {
     yylval.symbol = copy_str(yytext);
	return TOKEN_SHL;
}
shr {
     yylval.symbol = copy_str(yytext);
	return TOKEN_SHR;
}
ld {
     yylval.symbol = copy_str(yytext);
	return TOKEN_LD;
}
st {
     yylval.symbol = copy_str(yytext);
	return TOKEN_ST;
}
csrrd {
     yylval.symbol = copy_str(yytext);
	return TOKEN_CSRRD;
}
csrwr {
     yylval.symbol = copy_str(yytext);
	return TOKEN_CSRWR;
}
{GP_REGISTER} {
	if(!strcmp(yytext,"%sp")){
		yylval.num = 14;
		return TOKEN_GP_REGISTER;
	}
	if(!strcmp(yytext,"%pc")){
		yylval.num = 15;
		return TOKEN_GP_REGISTER;
	}
	yylval.num = atoi(yytext+2);
	return TOKEN_GP_REGISTER;
}
{CS_REGISTER} {
	yylval.symbol = copy_str(yytext+1);
	return TOKEN_CS_REGISTER;
}
{SYMBOL} {
	yylval.symbol = copy_str(yytext);
	return TOKEN_SYMBOL;
}

{COMMA} { return TOKEN_COMMA; }

\n             { return ENDL;}
. {}
%%

/* In the event of some kind of error, this will get called. */
int
yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}
