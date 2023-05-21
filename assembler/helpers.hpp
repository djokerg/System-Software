#include <vector>
#include "../assembler/directive.hpp"
#include "instruction.hpp"

struct arg{
	char* sym;
	int num = 0;//default, ne koristi se trenutno
	arg* next;
};

extern vector<Lang_Elem*>* list_of_lang_elems;
extern struct arg* global_arg;
extern FILE* myfile;
char* copy_str(const char*);

struct arg* mk_argument(char*);

void free_args(struct arg *args);
