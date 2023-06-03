#include "linker.hpp"

void Linker::initialize(string output_file, vector<string> input_files, map<string, int> sections_placed, bool is_relocatable)
{
  this->output_file = output_file;
  this->input_files = input_files;
  this->is_relocatable = is_relocatable;
  this->sections_placed = sections_placed;
}
Linker * Linker::instancePtr = nullptr;
Linker::Linker():linker_debugging_file("linker_debugging_file.txt"),current_aggregate_id(1){}

bool Linker::create_tables_from_input()
{

  for(string file :input_files){
    ifstream input_file(file, ios::binary);
    if(!input_file){
      errors_to_print.push_back(file +" doesnt exist");
      return false;
    }
    //symbol table restoration
    int num_of_entries;
    input_file.read((char*)(&num_of_entries), sizeof(num_of_entries));
    map<string, Symbol_table_entry> symbol_table;
    for(int i = 0; i < num_of_entries;i++){
      string key;
      Symbol_table_entry value;
      int length;
      input_file.read((char*)(&length), sizeof(length));
      key.resize(length);
      input_file.read((char*)key.c_str(), length);

      input_file.read((char*)(&value.defined), sizeof(value.defined));
      input_file.read((char*)(&value.global), sizeof(value.global));
      input_file.read((char*)(&value.id_temp), sizeof(value.id_temp));
      input_file.read((char*)(&value.is_extern), sizeof(value.is_extern));
      input_file.read((char*)(&value.value), sizeof(value.value));

      input_file.read((char*)(&length), sizeof(length));
      value.name.resize(length);
      input_file.read((char*)value.name.c_str(), length);

      input_file.read((char*)(&value.section), sizeof(int));

      symbol_table[key] = value;
      
    }
    symbol_tables[file] = symbol_table;
    //section table
    input_file.read((char*)(&num_of_entries), sizeof(num_of_entries));
    map<int, Section_table_entry> section_table;
    for(int i = 0; i < num_of_entries;i++){
      string key;
      Section_table_entry value;
      int length;
      input_file.read((char*)(&length), sizeof(length));
      key.resize(length);
      input_file.read((char*)key.c_str(), length);

      input_file.read((char*)(&value.section_id), sizeof(value.section_id));
      input_file.read((char*)(&value.size), sizeof(value.size));

      input_file.read((char*)(&length), sizeof(length));
      value.name.resize(length);
      input_file.read((char*)value.name.c_str(), length);

      value.virtual_address = 0;
      
      section_table[value.section_id] = value;
      
    }
    section_tables[file] = section_table;

    //relocation table
    input_file.read((char*)(&num_of_entries), sizeof(num_of_entries));
    map<string, vector<Reloc_table_entry>> relocation_table;
    for(int i = 0; i < num_of_entries;i++){
      string key;
      vector<Reloc_table_entry> value;
      int length;
      input_file.read((char*)(&length), sizeof(length));
      key.resize(length);
      input_file.read((char*)key.c_str(), length);
      
      int num_of_rows;
      input_file.read((char*)(&num_of_rows), sizeof(int));
      for(int i = 0; i < num_of_rows;i++){
        Reloc_table_entry entry;
        input_file.read((char*)(&entry.addend), sizeof(entry.addend));
        input_file.read((char*)(&entry.offset), sizeof(entry.offset));
      
        input_file.read((char*)(&length), sizeof(length));
        entry.symbol.resize(length);
        input_file.read((char*)entry.symbol.c_str(), length);

        value.push_back(entry);
      }
      relocation_table[key] = value;
    }
    relocation_tables[file] = relocation_table;
    //sections data

    input_file.read((char*)(&num_of_entries), sizeof(num_of_entries));
    map<string, stringstream*> data;
    for(int i =0; i < num_of_entries;i++){
      string key;
      int length;
      input_file.read((char*)(&length), sizeof(length));
      key.resize(length);
      input_file.read((char*)key.c_str(), length);

      string helper;
      input_file.read((char*)(&length), sizeof(length));
      helper.resize(length);
      input_file.read((char*)helper.c_str(), length);

      data[key] = new stringstream();
      data[key]->write(helper.c_str(), length);
    }
    datas[file] = data;
    input_file.close();
  }
  return true;
}

bool Linker::merge_same_name_sections()
{
  //add undefined section to aggregate table
  Aggregate_section und;
  und.aggregate_address = 0;
  und.aggregate_id = 0;
  und.aggregate_size = 0;
  merged_sections["UND"] = und;
  for(string file: input_files){
    //this has also to be in insertion order
    for(map<int, Section_table_entry>::iterator iter = section_tables[file].begin(); iter!= section_tables[file].end();iter++){
      if(iter->second.name != "UND"){//undefined section not in aggregate table
        map<string,Aggregate_section>::iterator iter2 = merged_sections.find(iter->second.name);
        if(iter2 != merged_sections.end()){
          //already exists, just add this section to agregate
          
        }else{
          //create new agregate
          Aggregate_section ag;
          ag.aggregate_id = current_aggregate_id++;
          ag.aggregate_address = 0;//this will be mapped
          ag.aggregate_size = 0;
          merged_sections[iter->second.name] = ag;
        }
        //add current section to aggregate sectio
        int offset_in_merged = merged_sections[iter->second.name].aggregate_size;
        merged_sections[iter->second.name].aggregate_size+=iter->second.size;
        merged_sections[iter->second.name].included_sections[file] = offset_in_merged;
      }
    }
  }
  return true;
}

bool Linker::map_aggregate_sections()
{
  //goal is to mapp aggregate sections from addres 0, but if i have place, i firstly place them, and after highest address, i place the rest of the sections
  //first iterate trouh placed sections
  //i need to sort aggregate_sections before this function
  //skip und section in every iteration
  int first_free_address = 0;
  for(map<string, int>::iterator iter = sections_placed.begin(); iter!=sections_placed.end();iter++){
    merged_sections[iter->first].aggregate_address = iter->second;//with this data i know all vitrual addresses of sections
    if(iter->second + merged_sections[iter->first].aggregate_size > first_free_address){
      first_free_address = iter->second + merged_sections[iter->first].aggregate_size;
    }
  }
  //check if there is some intersection between and two sections
  if(check_for_intersections()){
    return false;
  }
  //the rest of sections need to be sorted beofre iterating
  vector<pair<string,Aggregate_section>> mapVector(merged_sections.begin(),merged_sections.end());
  sort(mapVector.begin(), mapVector.end(), compareById);
  //now i have mapVector for iterating
  for(pair<string, Aggregate_section> iter: mapVector){
    if(iter.first == "UND"){
      //skip undefined section while mapping
      continue;
    }
    map<string,int>::iterator iter_placed = sections_placed.find(iter.first);
    if(iter_placed == sections_placed.end()){
      //update this section details
      merged_sections[iter.first].aggregate_address = first_free_address;
      first_free_address+=merged_sections[iter.first].aggregate_size;
    }
  }
  //all sections now have their virtual addressed,test this
  return true;
}
//returns true if there is intersection
bool Linker::check_for_intersections()
{
  for(map<string, Aggregate_section>::iterator iter = merged_sections.begin();iter!= merged_sections.end();iter++){
    //skip section which is not mapped already
    map<string,int>::iterator iter_placed = sections_placed.find(iter->first);
    if(iter_placed != sections_placed.end()){
      //section places, so inspect
      for(map<string, Aggregate_section>::iterator iter_inner = merged_sections.begin();iter_inner !=merged_sections.end();iter_inner++){
         map<string,int>::iterator iter_placed2 = sections_placed.find(iter_inner->first);
          if(iter_placed2 != sections_placed.end()){
            //section places, check if its the same section
            if(iter->first != iter_inner->first){
              //call function to check for intersection
              if(is_intersection(iter->second.aggregate_address, iter->second.aggregate_address+iter->second.aggregate_size
              ,iter_inner->second.aggregate_address, iter_inner->second.aggregate_address + iter_inner->second.aggregate_size)){
                errors_to_print.push_back("intersection between " + iter->first + " and " + iter_inner->first + " sections");
                return true;
              }
            }
          }
      }
    }
  }
  //all passed sucessfully
  return false;
}

bool Linker::is_intersection(int left1, int right1, int left2, int right2)
{
  return max(left1,left2) < min(right1, right2);
}

bool Linker::compareById(const pair<string, Aggregate_section>& a, const pair<string, Aggregate_section>& b)
{
  return a.second.aggregate_id < b.second.aggregate_id;
}

void Linker::print_aggregate_sections()
{
  linker_debugging_file << "#.sections" << endl;
  linker_debugging_file << "Address\t\tSize\tNdx\tName" << endl;
  for(map<string, Aggregate_section>::iterator iter = merged_sections.begin(); iter!=merged_sections.end();iter++){
    linker_debugging_file << hex << setfill('0') << setw(8) << (0xffffffff & iter->second.aggregate_address) << "\t";
    linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & iter->second.aggregate_size) << "\t";
    linker_debugging_file << dec;
    linker_debugging_file << iter->second.aggregate_id << "\t\t";
    linker_debugging_file << iter->first << endl;
  }
}
//leave for the end
void Linker::make_output_section_table()
{
  //first input all sections
  //this can depend of is it relocatable or not
}

bool Linker::merge_symbol_tables()
{
  int id_symbol = 0; 
  vector<pair<string,Aggregate_section>> mapVector(merged_sections.begin(),merged_sections.end());
  sort(mapVector.begin(), mapVector.end(), compareById);
  //now i have mapVector for iterating
  for(pair<string, Aggregate_section> iter: mapVector){
    //iterate trough them and put them to new symbol table
    Symbol_table_entry new_symbol;
    new_symbol.defined = true;
    new_symbol.global = false;
    new_symbol.id_temp = id_symbol++;
    new_symbol.name = iter.first;
    new_symbol.section = iter.second.aggregate_id;
    new_symbol.value = 0;//za sekciju je vrednost 0
    output_sym_table[new_symbol.name] = new_symbol;
  }
  map<string, Symbol_table_entry> extern_symbols;

  for(string file:input_files){
    map<string, Symbol_table_entry> temp_table = symbol_tables[file];
    for(map<string, Symbol_table_entry>::iterator iter = temp_table.begin(); iter!= temp_table.end();iter++){
      //look for it in aggregate sections to check if symbol is section
      map<string, Aggregate_section>::iterator iter_sec = merged_sections.find(iter->first);
      if(iter_sec == merged_sections.end()){
        //not a section
        // check if its extern
        if(iter->second.is_extern){
          extern_symbols[iter->first] = iter->second;
        }else{
          //check if symbol is already added to symbol_table
          map<string, Symbol_table_entry>::iterator iter_sym = output_sym_table.find(iter->first);
          if(iter_sym != output_sym_table.end() && iter->second.global){
            //double definition of symbol
            errors_to_print.push_back("Double definition of symbol " + iter->first);
            return false;
          }else{
            //add symbol to symbol table
            iter->second.id_temp = id_symbol++;
            if(iter->second.section != -1){
              //now i need offset of iter->section in aggregate section
              //i need section name to index aggregate sections
              //check this
              Aggregate_section as = merged_sections[section_tables[file][iter->second.section].name];
              iter->second.section = as.aggregate_id;
              iter->second.value = iter->second.value + as.included_sections[file];//there is offset of current sectin in which is symbol
            }
            output_sym_table[iter->first] = iter->second;
          }
        }
      }
    }
  }
  //try to determine if there are undefined extern symbols
  for(map<string, Symbol_table_entry>::iterator iter = extern_symbols.begin();iter!= extern_symbols.end();iter++){
    map<string, Symbol_table_entry>::iterator iter_sym = output_sym_table.find(iter->first);

    if(iter_sym != output_sym_table.end()){
      //found extern symbol
    }else{
      if(!is_relocatable){
        errors_to_print.push_back("Undefined symbol " + iter->first); 
        return false;
      }else{
        //just add this symbol to table, check its data
        iter->second.id_temp = id_symbol++;
        //all data is ok, section 0 is still undefined
        output_sym_table[iter->first] = iter->second;
      }
    }
  }
  return true;
}

void Linker::print_output_symbol_table()
{
  linker_debugging_file << "#.output_symtab" << endl;
    linker_debugging_file << "Num\tValue\tSize\tTYPE\tBind\tNdx\tName" << endl;
    map<string, Symbol_table_entry> symbol_table = output_sym_table;
    for(map<string, Symbol_table_entry>::iterator it = symbol_table.begin(); it != symbol_table.end(); it++){

      linker_debugging_file << it->second.id_temp << ":\t"; 
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & it->second.value) << "\t";
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
      linker_debugging_file << "NOTYPE\t";
      if(it->second.global == false){
        linker_debugging_file << "LOC\t\t";
      }else{
        if(it->second.defined == true){
          linker_debugging_file << "GLOB\t";
        }else{
          if(it->second.is_extern){
            linker_debugging_file << "GLOB\t";
          }
        }
      }
      linker_debugging_file << dec;
      linker_debugging_file << it->second.section << "\t\t";//id sekcije
      linker_debugging_file << it->second.name << endl;
    }
}

bool Linker::merge_relocation_tables()
{
  for(string file: input_files){
    map<string, vector<Reloc_table_entry>> reloc_table = relocation_tables[file];
    for(map<string, vector<Reloc_table_entry>>::iterator iter = reloc_table.begin(); iter!=reloc_table.end();iter++){
      //iterating trough all relocation tables for all sections
      for(Reloc_table_entry reloc : iter->second){
        Reloc_table_entry new_output;
        new_output.addend = reloc.addend;
        new_output.offset = reloc.offset + merged_sections[iter->first].included_sections[file];
        new_output.symbol = reloc.symbol;
        output_reloc_table[iter->first].push_back(new_output);
      }
    }
  }
  return true;
}

void Linker::print_output_relocation_table()
{
  for(map<string, vector<Reloc_table_entry>>::iterator it = output_reloc_table.begin(); it != output_reloc_table.end(); it++){
    linker_debugging_file << "#.reloc." << it->first << endl;
    linker_debugging_file << "Offset\tSymbol\tAddend" << endl;
    vector <Reloc_table_entry> reloc_table_vector = it->second;
    for(Reloc_table_entry relocs : reloc_table_vector){
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & relocs.offset) << "\t\t" << relocs.symbol << "\t\t" << setfill('0') << setw(4) << (0xffff & relocs.addend) << endl;
    }
    linker_debugging_file << dec;
  }
}

bool Linker::merge_section_data()
{
  for(string file: input_files){
    //iterate trough all data
    map<string, stringstream*> temp_data = datas[file];
    for(map<string,stringstream*>::iterator iter = temp_data.begin();iter!=temp_data.end();iter++){
      //all data should be ordered by insertion order
      map<string,stringstream*>::iterator finder = output_data.find(iter->first);
      if(finder != output_data.end()){
        string content = iter->second->str();
        output_data[iter->first]->write(content.c_str(), content.size());
      }
      else{
        output_data[iter->first] = new stringstream();
        string content = iter->second->str();
        output_data[iter->first]->write(content.c_str(), content.size());
      }
    }
  }
  return true;
}

void Linker::print_output_section_data()
{
  for(map<string, Aggregate_section>::iterator iter = merged_sections.begin(); iter != merged_sections.end();iter++){
      linker_debugging_file << "Section " << iter->first << ":"<<endl;
      if(iter->second.aggregate_size == 0){
        linker_debugging_file << "NO DATA" << endl;
      }else{

        linker_debugging_file << "num_of_bytes " << iter->second.aggregate_size << endl;
        int rows = (iter->second.aggregate_size-1)/8+1;

        for(int i = 0; i < rows;i++){
          linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & i*8) << ": ";
          for(int j = i*8; j < (i+1)*8;j++){
            char c;
            if(j<iter->second.aggregate_size){
              output_data[iter->first]->read(&c,sizeof(char));
            }
            else{
              c = 0;
            }
              linker_debugging_file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
              linker_debugging_file << dec;
          } 
          linker_debugging_file << endl;
        }
      } 
    } 
}

bool Linker::proceed_linking()
{
  //first create tables from input files
  if(!create_tables_from_input()){
    return false;
  }
  //print those tables 
  print_symbol_table();
  print_section_table();
  print_reloc_table();
  print_section_data();
  //mapping all sections
  if(!merge_same_name_sections()){
    return false;
  }
  if(!is_relocatable){
    if(!map_aggregate_sections()){
      return false;//dodati povratne vrednosti
    }
  }
  print_aggregate_sections();
  if(!merge_symbol_tables()){
    return false;
  }
  print_output_symbol_table();
  if(!merge_relocation_tables()){
    return false;
  }
  print_output_relocation_table();
   if(!merge_section_data()){
    return false;
  }
  print_output_section_data();
  //i am doing all operations as file will be relocatable
  return true;
}

void Linker::print_errors()
{
  cout << "Error messages:" << "\n";
  for(auto i : errors_to_print){
  cout << i << endl;
  }
}

void Linker::print_symbol_table()
{
  for(string file:input_files){
    linker_debugging_file << "#.symtab" << endl;
    linker_debugging_file << "Num\tValue\tSize\tTYPE\tBind\tNdx\tName" << endl;
    map<string, Symbol_table_entry> symbol_table = symbol_tables[file];
    for(map<string, Symbol_table_entry>::iterator it = symbol_table.begin(); it != symbol_table.end(); it++){

      linker_debugging_file << it->second.id_temp << ":\t"; 
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & it->second.value) << "\t";
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
      linker_debugging_file << "NOTYPE\t";
      if(it->second.global == false){
        linker_debugging_file << "LOC\t\t";
      }else{
        if(it->second.defined == true){
          linker_debugging_file << "GLOB\t";
        }else{
          if(it->second.is_extern){
            linker_debugging_file << "GLOB\t";
          }
        }
      }
      linker_debugging_file << dec;
      linker_debugging_file << it->second.section << "\t\t";//id sekcije
      linker_debugging_file << it->second.name << endl;
    }
  }
}

void Linker::print_section_table()
{
  for(string file:input_files){
    linker_debugging_file << "#.sections" << endl;
    linker_debugging_file << "Num\tValue\tSize\tTYPE\tBind\tNdx\tName" << endl;
    map<int, Section_table_entry> section_table = section_tables[file];
    for(map<int, Section_table_entry>::iterator it = section_table.begin(); it != section_table.end(); it++){

      linker_debugging_file << it->second.section_id << ":\t"; 
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & it->second.size) << "\t";
      linker_debugging_file << "SCTN\t";
      linker_debugging_file << "LOC\t\t\t";
      linker_debugging_file << dec;
      linker_debugging_file << it->second.section_id << "\t\t";
      linker_debugging_file << it->second.name << endl;
    }
  }
}

void Linker::print_reloc_table()
{
  for(string file:input_files){
    map<string,vector<Reloc_table_entry>> relocation_table = relocation_tables[file];
    for(map<string, vector<Reloc_table_entry>>::iterator it = relocation_table.begin(); it != relocation_table.end(); it++){
    linker_debugging_file << "#.reloc." << it->first << endl;
    linker_debugging_file << "Offset\tSymbol\tAddend" << endl;
    vector <Reloc_table_entry> reloc_table_vector = it->second;
    for(Reloc_table_entry relocs : reloc_table_vector){
      linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & relocs.offset) << "\t\t" << relocs.symbol << "\t\t" << setfill('0') << setw(4) << (0xffff & relocs.addend) << endl;
    }
    linker_debugging_file << dec;
  }
  }
}

void Linker::print_section_data()
{
  for(string file:input_files){
    map<int, Section_table_entry> section_table = section_tables[file];
    for(map<int, Section_table_entry>::iterator iter = section_table.begin(); iter != section_table.end();iter++){
      linker_debugging_file << "Section " << iter->second.name << ":"<<endl;
      if(iter->second.size == 0){
        linker_debugging_file << "NO DATA" << endl;
      }else{
        Section_table_entry entry = iter->second;

        linker_debugging_file << "num_of_bytes " << entry.size << endl;
        int rows = (entry.size-1)/8+1;

        for(int i = 0; i < rows;i++){
          linker_debugging_file << hex << setfill('0') << setw(4) << (0xffff & i*8) << ": ";
          for(int j = i*8; j < (i+1)*8;j++){
            char c;
            if(j<entry.size){
              
              datas[file].at(entry.name)->read(&c,sizeof(char));
            }
            else{
              c = 0;
            }
              linker_debugging_file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
              linker_debugging_file << dec;
          } 
          linker_debugging_file << endl;
        }
      } 
    } 
  }
}
