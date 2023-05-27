#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include "directive.hpp"

using namespace std;
class Directive;
class Instruction;
class Lang_Elem;
class Addressing;

class Assembler{
  friend class Directive;
  friend class Instruction;
  static Assembler* instancePtr;
  Assembler();
  bool first_pass();
  bool second_pass();

  struct Section_table_entry{
    string name;
    int size;
    int section_id;
    vector<int> data_start;
    vector<char> data;
  };

  struct Symbol_table_entry{
    string name;
    int value;
    bool global;
    bool is_extern;
    string section;
    int id_temp;
    bool defined;
  };

  struct Reloc_table_entry{
    int offset;
    string type;
    string symbol;
    string section_bind_to_reloc;
    int addend;
  };

  map<string, Symbol_table_entry> symbol_table;
  map<string, Section_table_entry> section_table;
  vector<Reloc_table_entry> relocation_table;

  int location_counter;
  string current_section;
  int current_id_symbol_table;
  int current_id_section_table;

  const char* input_file_path;
  const char* output_file_name;

  ofstream debugging_file;
  bool error_not_happened = true;

  bool process_label_first_pass(string label_name, int line_num);

  bool process_global_first_pass(string symbol_name, int line_num);

  bool process_extern_first_pass(string symbol_name, int line_num);

  bool process_section_first_pass(string section_name, int line_num);

  bool process_skip_first_pass(int value, int line);

  bool process_word_first_pass(pair<bool, Uni> argument, int line_num);
  
  bool process_ascii_first_pass(string text, int line);
  
  bool process_instruction_first_pass(string mnemonic, int line_num);

  bool process_end_directive(int line_num);

  bool process_skip_second_pass(int value);

  bool process_word_second_pass(pair<bool, Uni> argument, int line_num);

  bool process_ascii_second_pass(string argument, int line_num);//nije neophodan broj linije

  bool process_section_second_pass(string section);

  bool process_instruction_second_pass(string mnemonic, int line_n, Addressing* addr, int gpr1, int gpr2, char* csr);

  map<int, string> errors_to_print;//int shows line, string is message about that line

  bool finished_pass = false;
public:
  Assembler(const Assembler& obj) = delete;
  
  static Assembler* getInstance(){
    if(instancePtr == nullptr){
      instancePtr = new Assembler();
    }
    return instancePtr;
  }

  bool compile();

  void print_errors();

  void initialize(const char* input_file, const char* output_file){
    this->input_file_path = input_file;
    this->output_file_name = output_file;
  }

  void print_symbol_table();
  void print_section_table();
  void print_reloc_table();
  void print_sections_data();

};

#endif