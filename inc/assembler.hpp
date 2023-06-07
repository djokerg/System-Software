#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include "../inc/directive.hpp"
#include "../misc/lexer.hpp"
#include "../misc/parser.hpp"

using namespace std;
class Directive;
class Instruction;
class Lang_Elem;
class Addressing;
enum OP_CODE_MOD{
  HALT = 0b00000000,
  INT = 0b00010000,
  XCHG = 0b01000000,
  ADD = 0b01010000,
  SUB = 0b01010001,
  MUL = 0b01010010,
  DIV = 0b01010011,
  NOT = 0b01100000,
  AND = 0b01100001,
  OR = 0b01100010,
  XOR = 0b01100011,
  SHL = 0b01110000,
  SHR = 0b01110001,
  CSRRD = 0b10010000,
  CSRWR = 0b10010100,
  PUSH = 0b10000001,
  POP = 0b10010011,
  JMP_MEM = 0b00111000,
  JMP = 0b00110000,
  BEQ_MEM = 0b00111001,
  BEQ = 0b00110001,
  BNE_MEM = 0b00111010,
  BNE = 0b00110010,
  BGT_MEM = 0b00111011,
  BGT = 0b00110011,
  CALL_MEM = 0b00100001,
  CALL = 0b00100000,
  LD_MEM = 0b10010010,
  LD_GPR = 0b10010001,
  ST_MEM = 0b10000000,
  ST_MEM_MEM = 0b10000010,
  IRET_MEM_STATUS = 0b10010110
};

class Assembler{
  friend class Directive;
  friend class Instruction;
  static Assembler* instancePtr;
  Assembler();
  bool first_pass();
  bool second_pass();

  struct Literal_table_entry {
    int location;
  };

  struct Section_table_entry{
    string name;
    int size;
    int section_id;
    map<int, Literal_table_entry> literal_table;
    map<string, Literal_table_entry> symbol_l_table;
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
    int addend;
  };

  map<string, Symbol_table_entry> symbol_table;
  map<string, Section_table_entry> section_table;
  map<string, vector<Reloc_table_entry>> relocation_table;
  map<string, stringstream> data;
  int location_counter;
  string current_section;
  int current_id_symbol_table;
  int current_id_section_table;

  const char* input_file_path;
  const char* output_file_name;

  ofstream txt_object_file;

  void place_literal_pool();

  bool process_label_first_pass(string label_name, int line_num);

  bool process_global_first_pass(string symbol_name, int line_num);

  bool process_extern_first_pass(string symbol_name, int line_num);

  bool process_section_first_pass(string section_name, int line_num);

  bool process_skip_first_pass(int value, int line);

  bool process_word_first_pass(pair<bool, Uni> argument, int line_num);
  
  bool process_ascii_first_pass(string text, int line);
  
  bool process_instruction_first_pass(yytokentype intr_token,string mnemonic, int line_num, Addressing* addr);

  bool process_end_directive(int line_num);

  bool process_skip_second_pass(int value);

  bool process_word_second_pass(pair<bool, Uni> argument, int line_num);

  bool process_ascii_second_pass(string argument, int line_num);//nije neophodan broj linije

  bool process_section_second_pass(string section);

  int make_machine_instruction(OP_CODE_MOD opcode_mod, int a, int b, int c, int d);
  
  void make_bjmp_or_call(yytokentype instr_token, Addressing*addr, int gpr1, int gpr2);

  bool process_instruction_second_pass(yytokentype instr_token, string mnemonic, int line_n, Addressing* addr, int gpr1, int gpr2, int csr);
  
  void write_literal_pool();

  void create_relocation_entry(string symbol_name);

  void create_relocation_entry(string symbol_name,int offset);

  bool make_ld_st(yytokentype instr_token, Addressing*addr, int gpr1);

  void create_binary_file();

  bool process_global_second_pass(string symbol_name, int line_num);

  static bool compareById(pair<string,Symbol_table_entry>& a,pair<string,Symbol_table_entry>& b);

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
    string txt_output = output_file;
    txt_output = txt_output.substr(0, txt_output.size()-2) + "_txt.o";
    txt_object_file.open(txt_output);
  }

  void print_symbol_table();
  void print_section_table();
  void print_reloc_table();
  void print_sections_data();

};

#endif