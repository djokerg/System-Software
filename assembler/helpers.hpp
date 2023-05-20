#include <vector>
#include "../assembler/directive.hpp"

struct arg{
	char* sym;
	arg* next;
};

extern vector<Lang_Elem*>* list_of_lang_elems;
extern struct arg* global_arg;

char* copy_str(const char*);

struct arg* mk_argument(char*);

void free_args(struct arg *args);
