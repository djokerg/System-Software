#include "../inc/assembler.hpp"
#include <string>
#include <iostream>
using namespace std;
int main(int argc, char* argv[])
{
	const char* file_in;
	const char* file_out;
	string option = argv[1];
	if(option != "-o"){
		cout << "Output file not exitsts" << endl;
	}else{
		file_in = argv[3];
		file_out = argv[2];
	}

	Assembler* as = Assembler::getInstance();

	as->initialize(file_in, file_out);

	if(as->compile() == false){
		as->print_errors();
		return -1;
	}

	as->print_symbol_table();
	as->print_section_table();
	as->print_reloc_table();
	as->print_sections_data();
	cout << "sucess\n";
	
	return 0;
}
