/* If we want to use other functions, we have to put the relevant
 * header includes here. */
%{
	#include "helpers.hpp"
  #include "addressing.hpp"
	#include <stdio.h>
  #include <vector>
	int yylex(void);
	void yyerror(const char*);
  extern int line_num;
%}

/* These declare our output file names. */
%output "parser.cpp"
%defines "parser.hpp"
/* This union defines the possible return types of both lexer and
 * parser rules. We'll refer to these later on by the field name */
%union {
	int         num;
	char       *symbol;
	struct arg *arg;
  class Addressing* adr;
}

/* These define the tokens that we use in the lexer.
 * All of these have no meaningful return value. */
%token TOKEN_COMMA
%token ENDL
%token TOKEN_COMM
%token <symbol> TOKEN_GLOBAL
%token <symbol> TOKEN_EXTERN
%token <symbol> TOKEN_SECTION
%token <symbol> TOKEN_WORD
%token <symbol> TOKEN_SKIP
%token <symbol> TOKEN_ASCII
%token <symbol> TOKEN_EQU
%token <symbol> TOKEN_END
/* These are ALSO used in the lexer, but in addition to
 * being tokens, they also have return values associated
 * with them. We name those according to the names we used
 * above, in the %union declaration. So, the TOKEN_NUM
 * rule will return a value of the same type as num, which
 * (in this case) is an int. */
//%token <num>   TOKEN_NUM
%token <symbol> TOKEN_SYMBOL
%token <symbol> TOKEN_LABEL
%token <symbol> TOKEN_LITERAL
%token <symbol> TOKEN_STRING
/* These are non-terminals in our grammar, by which I mean, parser
 * rules down below. Each of these also has a meaningful return type,
 * which is declared in the same way. */
%token <symbol> TOKEN_HALT
%token <symbol> TOKEN_INT
%token <symbol> TOKEN_IRET
%token <symbol> TOKEN_CALL
%token <symbol> TOKEN_RET
%token <symbol> TOKEN_JMP
%token <symbol> TOKEN_BEQ
%token <symbol> TOKEN_BNE
%token <symbol> TOKEN_BGT
%token <symbol> TOKEN_PUSH
%token <symbol> TOKEN_POP
%token <symbol> TOKEN_XCHG
%token <symbol> TOKEN_ADD
%token <symbol> TOKEN_SUB
%token <symbol> TOKEN_MUL
%token <symbol> TOKEN_DIV
%token <symbol> TOKEN_NOT
%token <symbol> TOKEN_AND
%token <symbol> TOKEN_OR
%token <symbol> TOKEN_XOR
%token <symbol> TOKEN_SHL
%token <symbol> TOKEN_SHR
%token <symbol> TOKEN_LD
%token <symbol> TOKEN_ST
%token <symbol> TOKEN_CSRRD
%token <symbol> TOKEN_CSRWR


%token <symbol> TOKEN_GP_REGISTER
%token <symbol> TOKEN_CS_REGISTER

%token TOKEN_IMM;
%token TOKEN_LSQB;
%token TOKEN_RSQB;
%token TOKEN_PLUS;

%type <arg> list_symbol;
%type <arg> list_literal_or_symbol;
%type <adr> operand;//srediti ovo??
//%type <ident> rname;

%%

/* A program is defined recursively as either empty or an instruction
 * followed by another program. In this case, there's nothing meaningful
 * for us to do or return as an action, so we omit any action after the
 * rules. */

prog
: 
line ENDLS
| prog line ENDLS

  ;
  
ENDLS:
  ENDLS ENDL
  {
    line_num++;
  }
  | ENDL
  {
    line_num++;
  }
  ;


/* An instruction, in our toy assembly, is always an identifier (which
 * is the instruction name) and possibly arguments. The numbers in the
 * variable here refer to the position of the argument we want, and
 * will refer to either the result of the rule (in the case of other
 * parser rules) or the contents of yylval (in the case of lexer
 * tokens.) */
line
:
  label operation
  | label
  | operation
  | TOKEN_COMM
  ;

operation
: directive
| instruction
  ;

instruction:
  TOKEN_HALT
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,NULL,NULL));}
  | TOKEN_INT
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,NULL,NULL));}
  | TOKEN_IRET
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,NULL,NULL));}
  | TOKEN_CALL operand
  { list_of_lang_elems->push_back(new Instruction(line_num, $1, $2, NULL, NULL));}
  | TOKEN_RET
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,NULL,NULL));}
  | TOKEN_JMP operand
  { list_of_lang_elems->push_back(new Instruction(line_num, $1, $2, NULL, NULL));}
  | TOKEN_BEQ TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER TOKEN_COMMA operand
  { list_of_lang_elems->push_back(new Instruction(line_num, $1, $6, $2, $4));}
  | TOKEN_BNE TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER TOKEN_COMMA operand
  { list_of_lang_elems->push_back(new Instruction(line_num, $1, $6, $2, $4));}
  | TOKEN_BGT TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER TOKEN_COMMA operand
  { list_of_lang_elems->push_back(new Instruction(line_num, $1, $6, $2, $4));}
  | TOKEN_PUSH TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,NULL));}
  | TOKEN_POP TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,NULL));}
  | TOKEN_XCHG TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_ADD TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_SUB TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_MUL TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_DIV TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_NOT TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,NULL));}
  | TOKEN_AND TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_OR TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_XOR TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_SHL TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_SHR TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_LD operand TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1, $2, $4, NULL));}
  | TOKEN_ST TOKEN_GP_REGISTER TOKEN_COMMA operand
  { list_of_lang_elems->push_back(new Instruction(line_num, $1, $4, $2, NULL));}
  | TOKEN_CSRRD TOKEN_CS_REGISTER TOKEN_COMMA TOKEN_GP_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  | TOKEN_CSRWR TOKEN_GP_REGISTER TOKEN_COMMA TOKEN_CS_REGISTER
  { list_of_lang_elems->push_back(new Instruction(line_num, $1,NULL,$2,$4));}
  ;

operand:
  TOKEN_IMM TOKEN_LITERAL
  { $$ = new Addressing("imm", $2, NULL, NULL);}
  | TOKEN_IMM TOKEN_SYMBOL
  { $$ = new Addressing("imm", NULL, $2, NULL);}
  | TOKEN_LITERAL
  { $$ = new Addressing("memdir", $1, NULL, NULL);}//kod skoka ima drugacije znacenje
  | TOKEN_SYMBOL
  { $$ = new Addressing("memdir", NULL, $1, NULL);}//kod skoka ima drugacije znacenje, prepoznati da li je instrukcija skoka
  | TOKEN_GP_REGISTER
  { $$ = new Addressing("regdir", NULL, NULL, $1);}
  | TOKEN_LSQB TOKEN_GP_REGISTER TOKEN_RSQB 
  { $$ = new Addressing("regind", NULL, NULL, $2);}
  | TOKEN_LSQB TOKEN_CS_REGISTER TOKEN_RSQB
  { $$ = new Addressing("regind", NULL, NULL, $2);}
  | TOKEN_LSQB TOKEN_GP_REGISTER TOKEN_PLUS TOKEN_LITERAL TOKEN_RSQB 
  { $$ = new Addressing("regindpom", $4, NULL, $2);}
  | TOKEN_LSQB TOKEN_CS_REGISTER TOKEN_PLUS TOKEN_LITERAL TOKEN_RSQB
  { $$ = new Addressing("regindpom", $4, NULL, $2);}
  | TOKEN_LSQB TOKEN_GP_REGISTER TOKEN_PLUS TOKEN_SYMBOL TOKEN_RSQB
  { $$ = new Addressing("regindpom", NULL, $4, $2);}
  | TOKEN_LSQB TOKEN_CS_REGISTER TOKEN_PLUS TOKEN_SYMBOL TOKEN_RSQB
  { $$ = new Addressing("regindpom", NULL, $4, $2);}
  ;
label:
  TOKEN_LABEL
  {list_of_lang_elems->push_back(new Directive(line_num, $1, NULL));}
;

directive
: TOKEN_GLOBAL list_symbol
{  
  struct arg* arg_list = $2;
  vector<string>* arg_vector = new vector<string>();
  while(arg_list){
    arg_vector->push_back(arg_list->sym);
    arg_list= arg_list->next;
  }
  free_args(global_arg);
  global_arg = NULL;
  list_of_lang_elems->push_back(new Directive(line_num, $1, arg_vector));
  }
  | TOKEN_EXTERN list_symbol
  {
    struct arg* arg_list = $2;
  vector<string>* arg_vector = new vector<string>();
  while(arg_list){
    arg_vector->push_back(arg_list->sym);
    arg_list= arg_list->next;
  }
  free_args(global_arg);
  global_arg = NULL;
  list_of_lang_elems->push_back(new Directive(line_num, $1, arg_vector));
  }
  | TOKEN_SECTION TOKEN_SYMBOL
  {
    vector<string>* arg_vector = new vector<string>();
    arg_vector->push_back($2);
    list_of_lang_elems->push_back(new Directive(line_num, $1, arg_vector));
  }
  | TOKEN_WORD list_literal_or_symbol
  {
    struct arg* arg_list = $2;
  vector<string>* arg_vector = new vector<string>();
  while(arg_list){
    arg_vector->push_back(arg_list->sym);
    arg_list= arg_list->next;
  }
  free_args(global_arg);
  global_arg = NULL;
  list_of_lang_elems->push_back(new Directive(line_num, $1, arg_vector));
  }
  | TOKEN_SKIP TOKEN_LITERAL
  {
    vector<string>* arg_vector = new vector<string>();
    arg_vector->push_back($2);
    list_of_lang_elems->push_back(new Directive(line_num, $1, arg_vector));
  }
  | TOKEN_ASCII TOKEN_STRING
  {
    vector<string>* arg_vector = new vector<string>();
    arg_vector->push_back($2);
    list_of_lang_elems->push_back(new Directive(line_num, $1, arg_vector));
  }
  | TOKEN_END
  {
    list_of_lang_elems->push_back(new Directive(line_num, $1, NULL));
  }
;
/* An argument in this case has multiple choices: it can be a register
 * plus an offset, in which case it must be surrounded by parens, or
 * it can be just a register, in which case the parens are optional.
 *
 * The 'return value' of a rule is always stored in the $$ variable.
 * Here, I also name the results of terminals and non-terminals, instead
 * of addressing them by number.
 */


list_literal_or_symbol
: TOKEN_LITERAL 
{ 
  if(global_arg == NULL){
    global_arg = mk_argument($1);
  }else{
    global_arg->next = mk_argument($1);
    global_arg = global_arg->next;
  }
  $$ = global_arg;
}
| TOKEN_SYMBOL
{ 
  if(global_arg == NULL){
    global_arg = mk_argument($1);
  }else{
    global_arg->next = mk_argument($1);
    global_arg = global_arg->next;
  }
  $$ = global_arg;
}
| list_literal_or_symbol TOKEN_COMMA TOKEN_LITERAL 
{   
  if(global_arg == NULL){
    global_arg = mk_argument($3);
  }else{
    global_arg->next = mk_argument($3);
    global_arg = global_arg->next;
  }
}
| list_literal_or_symbol TOKEN_COMMA TOKEN_SYMBOL
{   
  if(global_arg == NULL){
    global_arg = mk_argument($3);
  }else{
    global_arg->next = mk_argument($3);
    global_arg = global_arg->next;
  }
}

list_symbol
: TOKEN_SYMBOL
{ 
  if(global_arg == NULL){
    global_arg = mk_argument($1);
  }else{
    global_arg->next = mk_argument($1);
    global_arg = global_arg->next;
  }
  $$ = global_arg;
}
| list_symbol TOKEN_COMMA TOKEN_SYMBOL
{   
  if(global_arg == NULL){
    global_arg = mk_argument($3);
  }else{
    global_arg->next = mk_argument($3);
    global_arg = global_arg->next;
  }
}
//ovo kod isntrukcija naci iz primera asemblera

/* arg
  : TOKEN_LPAR rname[tok] TOKEN_PLUS TOKEN_NUM[num] TOKEN_RPAR
    { $$ = mk_argument($tok, $num); }
  | TOKEN_LPAR TOKEN_NUM[num] TOKEN_PLUS rname[tok] TOKEN_RPAR
    { $$ = mk_argument($tok, $num); }
  | TOKEN_LPAR rname[tok] TOKEN_RPAR
    { $$ = mk_argument($tok, 0); }
  | rname[tok]
    { $$ = mk_argument($tok, 0); }
  ; */

/* In this case, I'm being a little bit obtuse: I should encode the
 * register name rule as a regular expression (i.e. all registers could
 * be identified by the regex r.*x) but instead I'm doing a check in the
 * body of the rule and returning out of the generated parser. */
/* rname
  : TOKEN_IDENT
    { if (!is_register($1)) {
		fprintf(stderr, "bad register name: %s\n", $1);
	  }
	  $$ = $1;
	}
  ; */

%%
