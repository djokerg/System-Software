#include "../inc/linker.hpp"

void Linker::initialize(string output_file, vector<string> input_files, map<string, unsigned int> sections_placed, bool is_relocatable)
{
  this->output_file = output_file;
  this->input_files = input_files;
  this->is_relocatable = is_relocatable;
  this->sections_placed = sections_placed;
  //make output file
  string txt_output;
  if(is_relocatable){
    txt_output = output_file.substr(0, output_file.size()-2) + "_txt.o";
  }else{
    txt_output = output_file.substr(0, output_file.size()-4) + "_txt.o";
  }
  txt_object_file.open(txt_output);
}
Linker * Linker::instancePtr = nullptr;
Linker::Linker():current_aggregate_id(1){}

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
  unsigned int first_free_address = 0;
  for(map<string, unsigned int>::iterator iter = sections_placed.begin(); iter!=sections_placed.end();iter++){
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
    map<string,unsigned int>::iterator iter_placed = sections_placed.find(iter.first);
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
    map<string,unsigned int>::iterator iter_placed = sections_placed.find(iter->first);
    if(iter_placed != sections_placed.end()){
      //section places, so inspect
      for(map<string, Aggregate_section>::iterator iter_inner = merged_sections.begin();iter_inner !=merged_sections.end();iter_inner++){
         map<string,unsigned int>::iterator iter_placed2 = sections_placed.find(iter_inner->first);
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

bool Linker::is_intersection(unsigned int left1, unsigned int right1, unsigned int left2, unsigned int right2)
{
  return max(left1,left2) < min(right1, right2);
}

bool Linker::compareById(const pair<string, Aggregate_section>& a, const pair<string, Aggregate_section>& b)
{
  return a.second.aggregate_id < b.second.aggregate_id;
}

bool Linker::compareBySymbolId(const pair<string, Symbol_table_entry> &a, const pair<string, Symbol_table_entry> &b)
{
  return a.second.id_temp < b.second.id_temp;
}
void Linker::print_aggregate_sections()
{
  txt_object_file << "#.output_sections" << endl;
  txt_object_file << "Address\t\tSize\tNdx\tName" << endl;
  for(map<string, Aggregate_section>::iterator iter = merged_sections.begin(); iter!=merged_sections.end();iter++){
    txt_object_file << hex << setfill('0') << setw(8) << (0xffffffff & iter->second.aggregate_address) << "\t";
    txt_object_file << hex << setfill('0') << setw(4) << (0xffff & iter->second.aggregate_size) << "\t";
    txt_object_file << dec;
    txt_object_file << iter->second.aggregate_id << "\t\t";
    txt_object_file << iter->first << endl;
  }
}
//leave for the end
void Linker::make_output_section_table()
{
  //first input all sections
  //this can depend of is it relocatable or not
  for(map<string, Aggregate_section>::iterator iter = merged_sections.begin();iter!=merged_sections.end();iter++){
    Section_table_entry new_entry;
    new_entry.name = iter->first;
    new_entry.section_id = iter->second.aggregate_id;
    new_entry.size = iter->second.aggregate_size;
    new_entry.virtual_address = iter->second.aggregate_address;
    output_sec_table[new_entry.section_id] = new_entry;
  }
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
    new_symbol.is_extern = false;
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
      if(iter_sec == merged_sections.end() && iter->second.global){
        //not a section
        // check if its extern
        if(iter->second.is_extern){
          extern_symbols[iter->first] = iter->second;
        }else{
          //check if symbol is already added to symbol_table
          map<string, Symbol_table_entry>::iterator iter_sym = output_sym_table.find(iter->first);
          if(iter_sym != output_sym_table.end() && iter->second.defined && iter_sym->second.defined){
            //double definition of symbol
            errors_to_print.push_back("Double definition of symbol " + iter->first);
            return false;
          }else if(iter->second.defined){
            //add symbol to symbol table if its defined
            iter->second.id_temp = id_symbol++;
            //now i need offset of iter->section in aggregate section
            //i need section name to index aggregate sections
            //check this
            Aggregate_section as = merged_sections[section_tables[file][iter->second.section].name];
            iter->second.section = as.aggregate_id;
            iter->second.value = iter->second.value + as.included_sections[file];//there is offset of current sectin in which is symbol
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
  //sort this table
  vector<pair<string,Symbol_table_entry>> mapVector(output_sym_table.begin(),output_sym_table.end());
  sort(mapVector.begin(), mapVector.end(), compareBySymbolId);
  //now i have mapVector for iterating
  txt_object_file << "#.output_symtab" << endl;
  txt_object_file << "Num\tValue\t\t\tTYPE\t\tBind\tNdx\tName" << endl;
  for(pair<string, Symbol_table_entry> iter: mapVector){
    txt_object_file << iter.second.id_temp << ":\t"; 
    txt_object_file << hex << setfill('0') << setw(8) << (iter.second.value) << "\t";
    map<string,Aggregate_section>::iterator iter_sections = merged_sections.find(iter.first);
    if(iter_sections != merged_sections.end()){
      txt_object_file << "SCTN\t\t";
    }else{
      txt_object_file << "NOTYPE\t";
    }
    if(iter.second.global == false){
      txt_object_file << "LOC\t\t";
    }else{
      if(iter.second.defined == true){
        txt_object_file << "GLOB\t";
      }else{
        if(iter.second.is_extern){
          txt_object_file << "GLOB\t";
        }
      }
    }
    txt_object_file << dec;
    txt_object_file << iter.second.section << "\t\t";//id sekcije
    txt_object_file << iter.second.name << endl;
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
        //add offset off in aggregate section, to have fully merged symbol table, if symbol is in fact section
        map<string, Aggregate_section>::iterator is_section = merged_sections.find(reloc.symbol);
        unsigned int additional_addend = 0;
        if(is_section != merged_sections.end()){
          additional_addend = merged_sections[reloc.symbol].included_sections[file];
        }
        new_output.addend = reloc.addend + additional_addend;//update addend if its local symbol
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
    txt_object_file << "#.reloc." << it->first << endl;
    txt_object_file << "Offset\tSymbol\tAddend" << endl;
    vector <Reloc_table_entry> reloc_table_vector = it->second;
    for(Reloc_table_entry relocs : reloc_table_vector){
      txt_object_file << hex << setfill('0') << setw(4) << (0xffff & relocs.offset) << "\t\t" << relocs.symbol << "\t\t" << setfill('0') << setw(4) << (0xffff & relocs.addend) << endl;
    }
    txt_object_file << dec;
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
      txt_object_file << "Section " << iter->first << ":"<<endl;
      if(iter->second.aggregate_size == 0){
        txt_object_file << "NO DATA" << endl;
      }else{

        txt_object_file << "num_of_bytes " << iter->second.aggregate_size << endl;
        int rows = (iter->second.aggregate_size-1)/8+1;

        for(int i = 0; i < rows;i++){
          txt_object_file << hex << setfill('0') << setw(8) << (0xffffffff & (i*8+iter->second.aggregate_address)) << ": ";
          for(int j = i*8; j < (i+1)*8;j++){
            char c;
            if(j<iter->second.aggregate_size){
              output_data[iter->first]->read(&c,sizeof(char));
            }
            else{
              c = 0;
            }
              txt_object_file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
              txt_object_file << dec;
          } 
          txt_object_file << endl;
        }
      } 
    } 
}

bool Linker::resolve_relocations_hex()
{
  //resolve first for all symbols, all symbols should have virtual addresses
  for(map<string, Symbol_table_entry>::iterator  iter = output_sym_table.begin(); iter!=output_sym_table.end();iter++){
    //check if symbol is section
    map<string, Aggregate_section>::iterator is_section = merged_sections.find(iter->first);
    if(is_section == merged_sections.end()){
      //not section, we can update value of symbol according to virtual address of section where they are
      iter->second.value += output_sec_table[iter->second.section].virtual_address;
    }
  }
  //now i can resolve all relocation_tables
  for(map<string, vector<Reloc_table_entry>>::iterator iter = output_reloc_table.begin();iter!=output_reloc_table.end();iter++){
    //now i have to find symbol which pass this allocation in some section
    for(Reloc_table_entry reloc : iter->second){
      unsigned int symbol_value = 0;
      map<string, Aggregate_section>::iterator is_section = merged_sections.find(reloc.symbol);
        if(is_section != merged_sections.end()){
          //it is section, so put its virual adress
          symbol_value = is_section->second.aggregate_address + reloc.addend;//this value is if its section in case
        }
        else{
          //find symbol in symbol table which have this specified name
          symbol_value = output_sym_table[reloc.symbol].value;
        }
        //now i have to go to this specified section data and write in position offset this symbol value
        stringstream* my_data = output_data[iter->first];
        //on postition 
        string current_data = my_data->str();
        string non_changable = current_data.substr(0, reloc.offset);
        string the_rest = current_data.substr(reloc.offset+4, current_data.size()-reloc.offset-4);
        my_data->seekp(0);
        my_data->write(non_changable.c_str(), non_changable.size());
        my_data->write((char*)(&symbol_value), sizeof(symbol_value));
        my_data->write(the_rest.c_str(), the_rest.size());
    }
  }
  return true;
}

void Linker::create_binary_file()
{
  ofstream binary_file(output_file, ios::out | ios::binary);
  if(is_relocatable){
    //write symbol table
    int num_of_entries = output_sym_table.size();
    binary_file.write((char*)(&num_of_entries), sizeof(int));
    for(map<string, Symbol_table_entry>::iterator iter = output_sym_table.begin(); iter != output_sym_table.end(); iter++){
      //output key first
      string key = iter->first;
      int length = key.length();
      binary_file.write((char*)(&length), sizeof(int));
      binary_file.write(key.c_str(), key.length());

      //output value
      binary_file.write((char*)(&iter->second.defined), sizeof(iter->second.defined));
      binary_file.write((char*)(&iter->second.global), sizeof(iter->second.global));
      binary_file.write((char*)(&iter->second.id_temp), sizeof(iter->second.id_temp));
      binary_file.write((char*)(&iter->second.is_extern), sizeof(iter->second.is_extern));
      binary_file.write((char*)(&iter->second.value), sizeof(iter->second.value));
      
      length = iter->second.name.length();
      binary_file.write((char*)(&length), sizeof(int));
      binary_file.write(iter->second.name.c_str(), iter->second.name.length());
      
      binary_file.write((char*)(&output_sec_table[iter->second.section].section_id), sizeof(int));
    
    }

    num_of_entries = output_sec_table.size();
    binary_file.write((char*)(&num_of_entries), sizeof(int));
    for(map<int, Section_table_entry>::iterator iter = output_sec_table.begin(); iter != output_sec_table.end(); iter++){
      //output key first
      string key = iter->second.name;
      int length = key.length();
      binary_file.write((char*)(&length), sizeof(int));
      binary_file.write(key.c_str(), key.length());

      //output value
      binary_file.write((char*)(&iter->second.section_id), sizeof(iter->second.section_id));
      binary_file.write((char*)(&iter->second.size), sizeof(iter->second.size));
      
      length = iter->second.name.length();
      binary_file.write((char*)(&length), sizeof(int));
      binary_file.write(iter->second.name.c_str(), iter->second.name.length());

      //literal table is not neccesary for linking
    }

    num_of_entries = output_reloc_table.size();
    binary_file.write((char*)(&num_of_entries), sizeof(int));
    for(map<string, vector<Reloc_table_entry>>::iterator iter = output_reloc_table.begin(); iter!=output_reloc_table.end(); iter++){
      //output key
      string key = iter->first;
      int length = key.length();
      binary_file.write((char*)(&length), sizeof(int));
      binary_file.write(key.c_str(), key.length());

      //output value
      int num_of_rows = iter->second.size();
      binary_file.write((char*)(&num_of_rows), sizeof(int));
      for(Reloc_table_entry reloc:iter->second){
        binary_file.write((char*)(&reloc.addend), sizeof(reloc.addend));
        binary_file.write((char*)(&reloc.offset), sizeof(reloc.offset));

        int length = reloc.symbol.length(); // edit this
        binary_file.write((char*)(&length), sizeof(int));
        binary_file.write(reloc.symbol.c_str(), reloc.symbol.length());
      }
    }
    //output of all sections data
    num_of_entries = output_data.size();
    binary_file.write((char*)(&num_of_entries), sizeof(int));
    for(map<string, stringstream*>::iterator iter = output_data.begin(); iter != output_data.end(); iter++){
      //output key first
      //patch this, you dont have to output length like this
      string key = iter->first;
      int length = key.length();
      binary_file.write((char*)(&length), sizeof(int));
      binary_file.write(key.c_str(), key.length());

      //output value
    
      iter->second->seekg(0);
      string data_output = iter->second->str();
      length = data_output.length();
      binary_file.write((char*)(&length), sizeof(length));
      binary_file.write(data_output.data(), data_output.length());
    }
  }else{
    //this is hex file, it contains only section table
    //dont count undefined section
    int num_of_sections = output_sec_table.size()-1;
    binary_file.write((char*)(&num_of_sections), sizeof(num_of_sections));
    for(map<int,Section_table_entry>::iterator iter = output_sec_table.begin(); iter!=output_sec_table.end();iter++){
      if(iter->second.name == "UND"){
        //this section has no size and need no memory
        continue;
      }
      int virtual_address_of_section = iter->second.virtual_address;
      binary_file.write((char*)(&virtual_address_of_section), sizeof(virtual_address_of_section));
      int section_size = iter->second.size;
      binary_file.write((char*)(&section_size), sizeof(section_size));
      string s_data = output_data[iter->second.name]->str();
      binary_file.write(s_data.c_str(), s_data.size());
    }
  }
  binary_file.close(); 
}

bool Linker::proceed_linking()
{
  //first create tables from input files
  if(!create_tables_from_input()){
    return false;
  }
  //print those tables 
  // print_symbol_table();
  // print_section_table();
  // print_reloc_table();
  // print_section_data();
  //mapping all sections
  if(!merge_same_name_sections()){
    return false;
  }
  if(!is_relocatable){
    if(!map_aggregate_sections()){
      return false;//dodati povratne vrednosti
    }
  }
  if(!merge_symbol_tables()){
    return false;
  }
  if(!merge_relocation_tables()){
    return false;
  }
   if(!merge_section_data()){
    return false;
  }
  make_output_section_table();
  if(!is_relocatable){
    if(!resolve_relocations_hex()){
      return false;
    }
  }
  print_output_symbol_table();
  print_aggregate_sections();
  print_output_relocation_table();
  print_output_section_data();
  //i am doing all operations as file will be relocatable
  create_binary_file();
  return true;
}

void Linker::print_errors()
{
  cout << "Linker\n";
  cout << "Error messages:" << "\n";
  for(auto i : errors_to_print){
  cout << i << endl;
  }
}

void Linker::print_symbol_table()
{
  for(string file:input_files){
    txt_object_file << "#.symtab" << endl;
    txt_object_file << "Num\tValue\tSize\tTYPE\tBind\tNdx\tName" << endl;
    map<string, Symbol_table_entry> symbol_table = symbol_tables[file];
    for(map<string, Symbol_table_entry>::iterator it = symbol_table.begin(); it != symbol_table.end(); it++){

      txt_object_file << it->second.id_temp << ":\t"; 
      txt_object_file << hex << setfill('0') << setw(4) << (0xffff & it->second.value) << "\t";
      txt_object_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
      txt_object_file << "NOTYPE\t";
      if(it->second.global == false){
        txt_object_file << "LOC\t\t";
      }else{
        if(it->second.defined == true){
          txt_object_file << "GLOB\t";
        }else{
          if(it->second.is_extern){
            txt_object_file << "GLOB\t";
          }
        }
      }
      txt_object_file << dec;
      txt_object_file << it->second.section << "\t\t";//id sekcije
      txt_object_file << it->second.name << endl;
    }
  }
}

void Linker::print_section_table()
{
  for(string file:input_files){
    txt_object_file << "#.sections" << endl;
    txt_object_file << "Num\tValue\tSize\tTYPE\tBind\tNdx\tName" << endl;
    map<int, Section_table_entry> section_table = section_tables[file];
    for(map<int, Section_table_entry>::iterator it = section_table.begin(); it != section_table.end(); it++){

      txt_object_file << it->second.section_id << ":\t"; 
      txt_object_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
      txt_object_file << hex << setfill('0') << setw(4) << (0xffff & it->second.size) << "\t";
      txt_object_file << "SCTN\t";
      txt_object_file << "LOC\t\t\t";
      txt_object_file << dec;
      txt_object_file << it->second.section_id << "\t\t";
      txt_object_file << it->second.name << endl;
    }
  }
}

void Linker::print_reloc_table()
{
  for(string file:input_files){
    map<string,vector<Reloc_table_entry>> relocation_table = relocation_tables[file];
    for(map<string, vector<Reloc_table_entry>>::iterator it = relocation_table.begin(); it != relocation_table.end(); it++){
    txt_object_file << "#.reloc." << it->first << endl;
    txt_object_file << "Offset\tSymbol\tAddend" << endl;
    vector <Reloc_table_entry> reloc_table_vector = it->second;
    for(Reloc_table_entry relocs : reloc_table_vector){
      txt_object_file << hex << setfill('0') << setw(4) << (0xffff & relocs.offset) << "\t\t" << relocs.symbol << "\t\t" << setfill('0') << setw(4) << (0xffff & relocs.addend) << endl;
    }
    txt_object_file << dec;
  }
  }
}

void Linker::print_section_data()
{
  for(string file:input_files){
    map<int, Section_table_entry> section_table = section_tables[file];
    for(map<int, Section_table_entry>::iterator iter = section_table.begin(); iter != section_table.end();iter++){
      txt_object_file << "Section " << iter->second.name << ":"<<endl;
      if(iter->second.size == 0){
        txt_object_file << "NO DATA" << endl;
      }else{
        Section_table_entry entry = iter->second;

        txt_object_file << "num_of_bytes " << entry.size << endl;
        int rows = (entry.size-1)/8+1;

        for(int i = 0; i < rows;i++){
          txt_object_file << hex << setfill('0') << setw(4) << (0xffff & i*8) << ": ";
          for(int j = i*8; j < (i+1)*8;j++){
            char c;
            if(j<entry.size){
              
              datas[file].at(entry.name)->read(&c,sizeof(char));
            }
            else{
              c = 0;
            }
              txt_object_file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
              txt_object_file << dec;
          } 
          txt_object_file << endl;
        }
      } 
    } 
  }
}
