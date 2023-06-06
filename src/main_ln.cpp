#include <regex>
#include <iostream>
#include "../inc/linker.hpp"
using namespace std;
int main(int argc, const char* argv[]){

  //loading command line arguments
  map<string, unsigned int> sections_to_be_placed;
  bool is_hex = false;
  bool is_relocatable = false;
  bool flag = false;
  vector<string> input_file_names;
  string output_file;
  regex place_section_regex("^-place=([a-zA-Z_][a-zA-Z_0-9]*)@(0[xX][0-9A-Fa-f]+)$");
  smatch matched_arguments;
  for(int i =1; i < argc;i++){
    string iter = argv[i];
    if(iter == "-o"){
      //need to set flag for picking up other files
      flag = true;
    }else if(regex_search(iter, matched_arguments, place_section_regex)){
      string section = matched_arguments.str(1);
      string address = matched_arguments.str(2);
      unsigned int section_addr = stol(address, nullptr, 16);
      sections_to_be_placed[section] = section_addr;
    }else if(iter == "-hex"){
      is_hex = true;
    }else if(iter == "-relocatable"){
      is_relocatable = true;
    }else if(flag){
      output_file = iter;
      flag = false;
    }else{
      input_file_names.push_back(iter);
    }
  }
  if(is_hex && is_relocatable){
    cout << "-hex and -relocatable options entered both";
    return -1;
  }

  if(!is_hex && !is_relocatable){
    cout << "one of the -hex or -relocatable options must be selected";
    return -1;
  }

  Linker* ln = Linker::getInstance();
  ln->initialize(output_file, input_file_names, sections_to_be_placed, is_relocatable);//if is relocatable, then no hex output

  if(!ln->proceed_linking()){
    ln->print_errors();
    return -1;
  }
  
  return 0;
}