#include "assembler.hpp"
#include "helpers.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "lang_elem.hpp"

Assembler::Assembler():location_counter(0), current_id_section_table(0), current_id_symbol_table(0), current_section(""), debugging_file("debugging_file.txt"){
	Section_table_entry undefined;
  undefined.name = "UNDEFINED";
  undefined.section_id = current_id_section_table++;
  undefined.size = 0;
	section_table["UNDEFINED"] = undefined;
}

bool Assembler::first_pass()
{
  for(int i =0; i < list_of_lang_elems->size(); i++){
    if(error_not_happened){
		  error_not_happened = list_of_lang_elems->at(i)->visit_first_pass();
    }
    if(finished_pass){
      return error_not_happened;
    }
	}
  return error_not_happened;
}

bool Assembler::second_pass()
{
  location_counter = 0;
  current_section = "";
  error_not_happened = true;
  for(int i =0; i < list_of_lang_elems->size(); i++){
    if(error_not_happened){
		  error_not_happened = list_of_lang_elems->at(i)->visit_second_pass();
    }
    if(finished_pass){
      return error_not_happened;
    }
	}
  return error_not_happened;
}

bool Assembler::process_label_first_pass(string label_name, int line_num)
{
  if(current_section == ""){
    errors_to_print[line_num] = "Label must be defined in section";
    return false;
  }
  map<string,Symbol_table_entry>::iterator iter = symbol_table.find(label_name);

	if( iter != symbol_table.end()){
		//error, double definition of label

    if(iter->second.defined == true){
		  errors_to_print[line_num] = "Double definition of a symbol";
      return false;
    }
    else if(iter->second.is_extern == true){
      errors_to_print[line_num] = "External symbol redefinition";
      return false;
    }

    iter->second.defined=true;
    iter->second.section = current_section;
    iter->second.value = location_counter;
	}
  else{
    Symbol_table_entry label_symbol;
    label_symbol.id_temp = current_id_symbol_table++;
    label_symbol.is_extern = false;
    label_symbol.global = false;
    label_symbol.defined = true;
    label_symbol.section = current_section;
    label_symbol.name = label_name;
    label_symbol.value = location_counter;
    symbol_table[label_name] = label_symbol;  
  }
  debugging_file << "Processed label " << label_name << endl;
  return true;
}

bool Assembler::process_global_first_pass(string symbol_name, int line_num)
{
	map<string,Symbol_table_entry>::iterator iter = symbol_table.find(symbol_name);

	if( iter != symbol_table.end()){
		iter->second.global = true;
	}else{
    Symbol_table_entry new_sym;
    new_sym.defined = false;
    new_sym.id_temp = current_id_symbol_table++;
    new_sym.global = true;
    new_sym.is_extern = false;
    new_sym.name = symbol_name;
    new_sym.section = "UNDEFINED";
    new_sym.value = 0;//nedefinisan
    symbol_table[symbol_name] = new_sym;
  }
  debugging_file << "Processed global symbol " << symbol_name << endl;
  return true;
}

void Assembler::print_symbol_table()
{
  //cuvam interno tabelu sekcija, ali u izlaznom fajlu ce biti objedinjeni
  debugging_file << "#.symtab" << endl;
  debugging_file << "Num\tValue\tSize\tTYPE\tBind\tNdx\tName" << endl;
  for(map<string, Section_table_entry>::iterator it = section_table.begin(); it != section_table.end(); it++){

    debugging_file << it->second.section_id << ":\t"; 
    debugging_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
    debugging_file << hex << setfill('0') << setw(4) << (0xffff & it->second.size) << "\t";
    debugging_file << "SCTN\t";
    debugging_file << "LOC\t\t\t";
    debugging_file << it->second.section_id << "\t\t";
    debugging_file << it->second.name << endl;
  }
  for(map<string, Symbol_table_entry>::iterator it = symbol_table.begin(); it != symbol_table.end(); it++){

    debugging_file << (it->second.id_temp+current_id_section_table) << ":\t"; 
    debugging_file << hex << setfill('0') << setw(4) << (0xffff & it->second.value) << "\t";
    debugging_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
    debugging_file << "NOTYPE\t";
    if(it->second.global == false){
      debugging_file << "LOC\t\t";
    }else{
      if(it->second.defined == true){
        debugging_file << "GLOB\t";
      }else{
        if(it->second.is_extern){
          debugging_file << "GLOB\t";
        }
      }
    }
    debugging_file << section_table[it->second.section].section_id << "\t\t";
    debugging_file << it->second.name << endl;
  }



}

void Assembler::print_section_table()
{

}

bool Assembler::process_extern_first_pass(string symbol_name, int line_num)
{

  map<string,Symbol_table_entry>::iterator iter = symbol_table.find(symbol_name);

	if( iter != symbol_table.end()){
    if(iter->second.defined == true){
      errors_to_print[line_num] = "Defined symbol cannot be declared as extern";
      return false;
    }
	}
  else{
    Symbol_table_entry new_sym;
    new_sym.defined = false;
    new_sym.global = false;
    new_sym.id_temp = current_id_symbol_table++;
    new_sym.is_extern = true;
    new_sym.name = symbol_name;
    new_sym.section = "UNDEFINED";
    new_sym.value = 0;
    symbol_table[symbol_name] = new_sym;
  }
  debugging_file << "Processed external symbol " << symbol_name << endl;
  return true;
}

bool Assembler::process_section_first_pass(string section_name, int line_num)
{
  if(current_section != ""){
    section_table[current_section].size = location_counter;
    debugging_file << "End of section " << current_section << endl;
  }
  location_counter = 0;
  current_section = section_name;
  Section_table_entry new_section;
  new_section.name = section_name;
  new_section.section_id = current_id_section_table++;
  new_section.size = 0;
  section_table[section_name] = new_section;
  debugging_file << "New section " << section_name << endl;
  return true;
}

bool Assembler::process_skip_first_pass(int value, int line)
{
  if(current_section == ""){
    //not in any section
    errors_to_print[line] = "Skip directive must be used in a section!";
    return false;
  }
  else{
    location_counter += value;
    debugging_file << "Skipped " << value << " bytes-first pass" << endl;
    return true;
  }
  
}

bool Assembler::compile()
{

  list_of_lang_elems = new vector<Lang_Elem*>();
	global_arg = nullptr;
	myfile = fopen(this->input_file_path, "r");
	if (!myfile) {
  	cout << "I can't open file!" << endl;
  	return false;
	}
	yyin = myfile;
	if (yyparse())
		return false;//not regular
	
	for(int i =0; i < list_of_lang_elems->size(); i++){
		list_of_lang_elems->at(i)->print_le();
	}
  // for(int i =0; i < list_of_lang_elems->size(); i++){
	// 	list_of_lang_elems->at(i)->visit();
	// }

  if(first_pass() == false) return false;
  //if(second_pass() == false) return false;
  return true;
}

bool Assembler::process_word_first_pass(pair<bool, Uni> argument, int line_num)
{
  //uvecam location counter po 4 bajta za svaki inicijalizator
  if(current_section == ""){
    errors_to_print[line_num] = "Word directive must be used in section!";
    return false;
  }
  
  location_counter+=4;
  debugging_file << "Reserved 4 bytes for word argument, first pass" << endl;
  return true;

}
Assembler *Assembler::instancePtr = nullptr;

void Assembler::print_errors()
{
  cout << "Error messages:" << "\n";
  for(auto i : errors_to_print){
    cout << "Error on line "<< i.first << "-" << i.second << endl;
  }
}

bool Assembler::process_ascii_first_pass(string text, int line)
{
  if(current_section == ""){
    errors_to_print[line] = "Ascii directive must be used in section!";
  }
  location_counter+=text.size();
  debugging_file << "Processed ascii first pass with text " << text << endl;
  return true;
}

bool Assembler::process_instruction_first_pass(string mnemonic,int line_num)
{
  if(current_section == ""){
    errors_to_print[line_num] = "Invalid instruction " + mnemonic + " outside of a section!";
    return false;
  }
  else{
    location_counter+=4;
    debugging_file << "Processed instruction " << mnemonic << " first pass" << endl;
    return true;
  }
}

bool Assembler::process_end_directive(int line_num)
{
  if(current_section == ""){
    errors_to_print[line_num] = "End directive out of any section!";
    return false;
  }
  section_table[current_section].size = location_counter;
  finished_pass = true;
  debugging_file << "End of an assembler pass" << endl;
  return true;
}

bool Assembler::process_skip_second_pass(int value)
{
  section_table[current_section].data_start.push_back(location_counter);
  for(int i =0; i < value; i++){
    section_table[current_section].data.push_back(0);
  }
  location_counter += value;
  return true;
}
