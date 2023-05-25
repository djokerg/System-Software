#include <vector>
#include "directive.hpp"
#include "instruction.hpp"
//helper fajl za lekser i parser. sve potrebno se nalazi u listi u main datoteci
struct arg{
	char* sym;
	int num;//default, ne koristi se trenutno
	arg* next;
};

extern vector<Lang_Elem*>* list_of_lang_elems;
extern struct arg* global_arg;
extern FILE* myfile;
char* copy_str(const char*);

struct arg* mk_argument(char*,int);

void free_args(struct arg *args);
