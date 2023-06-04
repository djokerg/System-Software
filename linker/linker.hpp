#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

class Linker{
  static Linker* instancePtr;
  Linker();
  string output_file;
  vector<string> input_files;
  map<string, int> sections_placed;
  bool is_relocatable;
  ofstream linker_debugging_file;
  vector<string> errors_to_print;
  
  //strukture slicne kao u asebleru

  struct Literal_table_entry {
    int location;
  };

  struct Section_table_entry{
    string name;
    int size;
    int section_id;
    int virtual_address;
  };

  struct Symbol_table_entry{
    string name;
    int value;
    bool global;
    bool is_extern;
    int section;
    int id_temp;
    bool defined;
  };

  struct Reloc_table_entry{
    int offset;
    //no type provided, delete it from original in assembler
    string symbol;
    int addend;
  };

  struct Aggregate_section{
    int aggregate_id;
    int aggregate_size;
    int aggregate_address;
    map<string, int> included_sections;//pair file, offset in merged sections
  };

  int current_aggregate_id;

  map<string, map<string, Symbol_table_entry>> symbol_tables;
  map<string, map<int, Section_table_entry>> section_tables;
  map<string,map<string, vector<Reloc_table_entry>>> relocation_tables;
  map<string, map<string, stringstream*>> datas;
  
  map<string, Aggregate_section> merged_sections;

  map<string, Symbol_table_entry> output_sym_table;
  map<int, Section_table_entry> output_sec_table;
  map<string, vector<Reloc_table_entry>> output_reloc_table;
  map<string, stringstream*> output_data;

  bool create_tables_from_input();
  bool merge_same_name_sections();
  bool map_aggregate_sections();
  bool check_for_intersections();
  bool is_intersection(int left1, int right1, int left2, int right2);
  static bool compareById(const pair<string,Aggregate_section> &a, const pair<string,Aggregate_section> &b);
  void print_aggregate_sections();
  void make_output_section_table();
  bool merge_symbol_tables();
  void print_output_symbol_table();
  bool merge_relocation_tables();
  void print_output_relocation_table();
  bool merge_section_data();
  void print_output_section_data();
  bool resolve_relocations_hex();
  void create_binary_file();
public:
  Linker(const Linker& obj) = delete;
  
  static Linker* getInstance(){
    if(instancePtr == nullptr){
      instancePtr = new Linker();
    }
    return instancePtr;
  }

  void initialize(string output_file, vector<string> input_files, map<string, int> sections_placed, bool is_relocatable);

  bool proceed_linking();

  void print_errors();

  void print_symbol_table();

  void print_section_table();

  void print_reloc_table();

  void print_section_data();

};