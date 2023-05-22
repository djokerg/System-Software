#include "helpers.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_IDENT_SIZE 64

vector<Lang_Elem*>* list_of_lang_elems = NULL;
struct arg* global_arg;

char*
copy_str(const char *in)
{
	size_t len = strnlen(in, MAX_IDENT_SIZE);
	char* buf = (char*)malloc(len + 1);
	strncpy(buf, in, len);
	buf[len] = '\0';
	return buf;
}

struct arg*
mk_argument(char *sym,int num)
{
	struct arg* a = (arg*)malloc(sizeof(struct arg));
	a->sym = sym;
	a->num = num;
	a->next = nullptr;
	return a;
}
void
free_args(struct arg *args)
{
	if (!args) return;
	while(args){
		arg* tmp = args;
		args = args->next;
		free(tmp);
	}
	global_arg = NULL;
}
FILE* myfile = nullptr;
// Open a file handle to a particular file:
