#include "helpers.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <stdio.h>

int
main(int argc, char* argv[])
{
	//izmeniti ovaj fajl tako da on predstavlja samo pokretanje asembler programa
	list_of_lang_elems = new vector<Lang_Elem*>();
	global_arg = NULL;
	/* The default behaviour of yyparse will call the lexer and
     * then the parser on the contents of stdin. In our case, we're
     * not returning anything; we'll have a pointer to the last parsed
     * inside `global_prev`. */
	if (yyparse())
		return 1;
	for(int i =0; i < list_of_lang_elems->size(); i++){
		Directive* dir = (Directive*)list_of_lang_elems->at(i);
		dir->print_directive();
	}
	return 0;
}
