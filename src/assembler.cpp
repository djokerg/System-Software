#include "../inc/assembler.hpp"
#include "../inc/helpers.hpp"
#include "../inc/lang_elem.hpp"
#include "../inc/instruction.hpp"
#include "../inc/addressing.hpp"

Assembler::Assembler():location_counter(0), current_id_section_table(0), current_id_symbol_table(0), current_section(""){
	Section_table_entry undefined;
  undefined.name = "UND";
  undefined.section_id = current_id_section_table++;
  undefined.size = 0;
	section_table["UND"] = undefined;

  data["UND"] = stringstream();

  Symbol_table_entry new_symbol;
  new_symbol.defined = true;
  new_symbol.global = false;
  new_symbol.id_temp = current_id_symbol_table++;
  new_symbol.is_extern = false;
  new_symbol.name = "UND";
  new_symbol.section = "UND";
  new_symbol.value = 0;
  symbol_table["UND"] = new_symbol;
}

bool Assembler::first_pass()
{
  //return true if error not happened
  bool error_not_happened = true;
  for(int i =0; i < list_of_lang_elems->size(); i++){
    bool error = list_of_lang_elems->at(i)->visit_first_pass();
    if(error_not_happened){
      error_not_happened = error;
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
  finished_pass = false;
  bool error_not_happened = true;
  for(int i =0; i < list_of_lang_elems->size(); i++){
    bool error = list_of_lang_elems->at(i)->visit_second_pass();
    if(error_not_happened){
      error_not_happened = error;
    }
    if(finished_pass){
      return error_not_happened;
    }
	}
  return error_not_happened;
}

void Assembler::place_literal_pool()
{
  //iterating trough sections
  for(map<string, Section_table_entry>::iterator iter = section_table.begin(); iter != section_table.end(); iter++){
    //iterating trough all literals in pool
    for(map<int, Literal_table_entry>::iterator iter2 = iter->second.literal_table.begin(); iter2 != iter->second.literal_table.end(); iter2++){
      //iterating trough all literals in literal pool
      iter2->second.location = iter->second.size;
      iter->second.size+=4;
    }
  }
}

void Assembler::write_literal_pool()
{
  //iterating trough sections
  for(map<string, Section_table_entry>::iterator iter = section_table.begin(); iter != section_table.end(); iter++){
    //iterating trough all literals in pool
    for(map<int, Literal_table_entry>::iterator iter2 = iter->second.literal_table.begin(); iter2 != iter->second.literal_table.end(); iter2++){
      //iterating trough all literals in literal pool
      int value = iter2->first;
      data[iter->second.name].write((char*)(&value), sizeof(int));
    }
    for(map<string, Literal_table_entry>::iterator iter2 = iter->second.symbol_l_table.begin(); iter2 != iter->second.symbol_l_table.end(); iter2++){
      //iterating trough all symbols in symbol pool
      //do something different
      int value = 0;
      data[iter->second.name].write((char*)(&value), sizeof(int));//za svaki simbol jednostavno nulu ipisati.
    }
  }
}
//creating relocation on current location counter
void Assembler::create_relocation_entry(string symbol_name)
{
  map<string, Symbol_table_entry>::iterator iter = symbol_table.find(symbol_name);
    if(iter != symbol_table.end()){
      //entry mi predstavlja simbol na osnovu koga patchujem relokaciju
      Symbol_table_entry entry = iter->second;
      if(entry.defined){
        if(!entry.global){
          //symbol is local, so we put section for symbol name
          int c = 0;
          Reloc_table_entry new_reloc;
          new_reloc.addend = entry.value;//vrednost simbola koji je lokalan ide u addend, na osnovu njega vrsim prepravku
          new_reloc.offset = location_counter;
          new_reloc.symbol = entry.section;
          new_reloc.type = "R_TYPE";
          relocation_table[current_section].push_back(new_reloc);
        } 
        else{
          //global symbol
          Reloc_table_entry new_reloc;
          new_reloc.addend = 0;//vrednost simbola koji je lokalan ide u addend, na osnovu njega vrsim prepravku
          new_reloc.offset = location_counter;
          new_reloc.symbol = entry.name;
          new_reloc.type = "R_TYPE";
          relocation_table[current_section].push_back(new_reloc);
        }
      }else{
        //external symbol
        Reloc_table_entry new_reloc;
        new_reloc.addend = 0;//vrednost simbola koji je lokalan ide u addend, na osnovu njega vrsim prepravku
        new_reloc.offset = location_counter;
        new_reloc.symbol = entry.name;
        new_reloc.type = "R_TYPE";
        relocation_table[current_section].push_back(new_reloc);
      }
    }
}

void Assembler::create_relocation_entry(string symbol_name, int offset)
{
  //check if its already in relocation_table
  for(int i =0; i < relocation_table[current_section].size(); i++){
    if(relocation_table[current_section].at(i).symbol == symbol_name && relocation_table[current_section].at(i).offset == offset){
      return;
    }
  }
  map<string, Symbol_table_entry>::iterator iter = symbol_table.find(symbol_name);
    if(iter != symbol_table.end()){
      //entry mi predstavlja simbol na osnovu koga patchujem relokaciju
      Symbol_table_entry entry = iter->second;
      if(entry.defined){
        if(!entry.global){
          //symbol is local, so we put section for symbol name
          int c = 0;
          Reloc_table_entry new_reloc;
          new_reloc.addend = entry.value;//vrednost simbola koji je lokalan ide u addend, na osnovu njega vrsim prepravku
          new_reloc.offset = offset;
          new_reloc.symbol = entry.section;
          new_reloc.type = "R_TYPE";
          relocation_table[current_section].push_back(new_reloc);
        } 
        else{
          //global symbol
          Reloc_table_entry new_reloc;
          new_reloc.addend = 0;//vrednost simbola koji je lokalan ide u addend, na osnovu njega vrsim prepravku
          new_reloc.offset = offset;
          new_reloc.symbol = entry.name;
          new_reloc.type = "R_TYPE";
          relocation_table[current_section].push_back(new_reloc);
        }
      }else{
        //external symbol
        Reloc_table_entry new_reloc;
        new_reloc.addend = 0;//vrednost simbola koji je lokalan ide u addend, na osnovu njega vrsim prepravku
        new_reloc.offset = offset;
        new_reloc.symbol = entry.name;
        new_reloc.type = "R_TYPE";
        relocation_table[current_section].push_back(new_reloc);
      }
    }
}

bool Assembler::make_ld_st(yytokentype instr_token, Addressing *addr, int gpr1)
{
  OP_CODE_MOD opcode;
  int instr = 0;
  int instr2 = 0;
  //store ima dva koda operacije
  //kako bi mogla store ovde da se uglavi?
  switch(instr_token){
    case TOKEN_LD:{
      switch(addr->type){
        case MEMDIR:{
          opcode = LD_MEM;
          //add instruction which load from specified register to specified register(again the same)
          instr2 = make_machine_instruction(opcode, gpr1 ,gpr1, 0, 0);
        }
        case IMMED:{
          opcode = LD_MEM;
          int d = 0;
          if(!addr->symbol){
            d = section_table[current_section].literal_table[addr->literal].location - location_counter - 4;
          }else{
            map<string, Literal_table_entry>& table = section_table[current_section].symbol_l_table;
            map<string, Literal_table_entry>::iterator iter = table.find(addr->symbol);
            int reloc_location = 0;
            if(iter != table.end()){
              //found symobl, take location and place it to displacement
              d = iter->second.location - location_counter -4;
              reloc_location = iter->second.location;
            }else{
              //symbol not found in pool, add it
              Literal_table_entry new_symbol;
              new_symbol.location = section_table[current_section].size;
              section_table[current_section].size+= 4;//uvecavam je svesno
              table[addr->symbol] = new_symbol;

              d = new_symbol.location - location_counter -4;
              reloc_location = new_symbol.location;
            }
            //create rellocation
            create_relocation_entry(addr->symbol, reloc_location);
          }
          instr = make_machine_instruction(opcode, gpr1 ,15, 0, d);
          break;
        }
        //no literal pool for these instructions
        case REGDIR:{
          opcode = LD_GPR;
          instr = make_machine_instruction(opcode, gpr1, addr->gpr, 0,0);
          break;
        }
        case REGIND:{
          opcode = LD_MEM;
          instr = make_machine_instruction(opcode, gpr1, addr->gpr, 0, 0);
          break;
        }
        case REGINDPOM:{
          opcode = LD_MEM;
          int displacement = 0;
          if(addr->symbol){
            //there i can return false immediately
            return false;
          }else{
            displacement = addr->literal;
          }
          if(displacement > 1<<11 || displacement < -(1<<11)){
            return false;
          }
          instr = make_machine_instruction(opcode, gpr1, addr->gpr, 0, displacement);
          break;
        }
      }
      break;
    }
    case TOKEN_ST:{
      switch(addr->type){
        case IMMED: return false;
        case MEMDIR: {
          opcode = ST_MEM_MEM; 
          int d = 0;
          if(!addr->symbol){
                d = section_table[current_section].literal_table[addr->literal].location - location_counter - 4;
          }else{
            map<string, Literal_table_entry>& table = section_table[current_section].symbol_l_table;
            map<string, Literal_table_entry>::iterator iter = table.find(addr->symbol);
            int reloc_location = 0;
            if(iter != table.end()){
              //found symobl, take location and place it to displacement
              d = iter->second.location - location_counter -4;
              reloc_location = iter->second.location;
            }else{
              //symbol not found in pool, add it
              Literal_table_entry new_symbol;
              new_symbol.location = section_table[current_section].size;
              section_table[current_section].size+= 4;//uvecavam je svesno
              table[addr->symbol] = new_symbol;

              d = new_symbol.location - location_counter -4;
              reloc_location = new_symbol.location;
            }
            //create rellocation
            create_relocation_entry(addr->symbol, reloc_location);
          }
          instr = make_machine_instruction(opcode, 15, 0, gpr1, d);
          break;
        }
        case REGDIR:{
          opcode = LD_GPR;
          instr = make_machine_instruction(opcode, addr->gpr, gpr1, 0,0);
          break;
        }
        case REGIND:{
          opcode = ST_MEM;
          instr = make_machine_instruction(opcode, addr->gpr, 0, gpr1, 0);
          break;
        }
        case REGINDPOM:{
          opcode = ST_MEM;
          int displacement = 0;
          if(addr->symbol){
            if(symbol_table[addr->symbol].section == "ABSOLUTE"){
              displacement = symbol_table[addr->symbol].value;
            }else{
              return false;
            }
          }else{
            displacement = addr->literal;
          }
          if(displacement & (~(1<<12 - 1))){
            return false;
          }
          instr = make_machine_instruction(opcode, addr->gpr, 0, gpr1, displacement);
          break;
        }
      }
    }
  }
  data[current_section].write((char*)(&instr), sizeof(int));
  if(instr2){
    data[current_section].write((char*)(&instr2), sizeof(int));
  }
  return true;
}

void Assembler::create_binary_file()
{
  ofstream binary_file(output_file_name, ios::out | ios::binary);

  //write symbol table

  int num_of_entries = symbol_table.size();
  binary_file.write((char*)(&num_of_entries), sizeof(int));
  for(map<string, Symbol_table_entry>::iterator iter = symbol_table.begin(); iter != symbol_table.end(); iter++){
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
    
    binary_file.write((char*)(&section_table[iter->second.section].section_id), sizeof(int));
  
  }

  num_of_entries = section_table.size();
  binary_file.write((char*)(&num_of_entries), sizeof(int));
  for(map<string, Section_table_entry>::iterator iter = section_table.begin(); iter != section_table.end(); iter++){
    //output key first
    string key = iter->first;
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

  num_of_entries = relocation_table.size();
  binary_file.write((char*)(&num_of_entries), sizeof(int));
  for(map<string, vector<Reloc_table_entry>>::iterator iter = relocation_table.begin(); iter!=relocation_table.end(); iter++){
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
  num_of_entries = data.size();
  binary_file.write((char*)(&num_of_entries), sizeof(int));
  for(map<string, stringstream>::iterator iter = data.begin(); iter != data.end(); iter++){
    //output key first
    //patch this, you dont have to output length like this
    string key = iter->first;
    int length = key.length();
    binary_file.write((char*)(&length), sizeof(int));
    binary_file.write(key.c_str(), key.length());

    //output value
  
    iter->second.seekg(0);
    string data_output = iter->second.str();
    length = data_output.length();
    binary_file.write((char*)(&length), sizeof(length));
    binary_file.write(data_output.data(), data_output.length());
  }
  binary_file.close();
}

bool Assembler::process_global_second_pass(string symbol_name, int line_num)
{
  if(!symbol_table[symbol_name].defined){
    errors_to_print[line_num] = "Symbol " + symbol_name + " not defined, but declared as global.";
  }
  return true;
}
bool Assembler::compareById(pair<string, Symbol_table_entry> &a, pair<string, Symbol_table_entry> &b)
{
  return a.second.id_temp < b.second.id_temp;
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
    new_sym.section = "UND";
    new_sym.value = 0;//nedefinisan
    symbol_table[symbol_name] = new_sym;
  }
  return true;
}

int Assembler::make_machine_instruction(OP_CODE_MOD opcode_mod, int a = 0, int b = 0, int c = 0, int d = 0)
{
  int first = opcode_mod<<24;
  int second = ((a<<4 & 0xf0)|(b & 0xf))<<16;
  int third = ((c<<4 & 0xf0)|(d >> 8 & 0xf))<<8;
  int fourth = d & 0xff;
  int instr = first|second|third|fourth;
  return instr;
}

void Assembler::make_bjmp_or_call(yytokentype instr_token, Addressing *addr, int gpr1, int gpr2)
{
  OP_CODE_MOD opcode1;
  OP_CODE_MOD opcode2;
  switch(instr_token){
    case TOKEN_JMP:{
      opcode1 = JMP_MEM;
      opcode2 = JMP;
      break;
    }
    case TOKEN_CALL:{
      opcode1 = CALL_MEM;
      opcode2 = CALL;
      break;
    }
    case TOKEN_BEQ:{
      opcode1 = BEQ_MEM;
      opcode2 = BEQ;
      break;
    }
    case TOKEN_BNE:{
      opcode1 = BNE_MEM;
      opcode2 = BNE;
      break;
    }
    case TOKEN_BGT:{
      opcode1 = BGT_MEM;
      opcode2 = BGT;
      break;
    }
  }
  int instr = 0;
  if(!addr->symbol){
    int d = section_table[current_section].literal_table[addr->literal].location - location_counter - 4;
    instr = make_machine_instruction(opcode1, 15 ,gpr1, gpr2, d);
  }else{
    //check if its in this section
    if(symbol_table[addr->symbol].section == current_section){
      //found symbol, make instruction with direct displacement
      int d = symbol_table[addr->symbol].value - location_counter -4;
      instr = make_machine_instruction(opcode2, 15 , gpr1, gpr2, d);
    }else{
      map<string, Literal_table_entry>& table = section_table[current_section].symbol_l_table;
      map<string, Literal_table_entry>::iterator iter = table.find(addr->symbol);
      int reloc_location = 0;
      if(iter != table.end()){
        //found symobl, take location and place it to displacement
        int d = iter->second.location - location_counter -4;
        instr = make_machine_instruction(opcode1, 15 ,gpr1, gpr2, d);
        reloc_location = iter->second.location;
      }else{
        //symbol not found in pool, add it
        Literal_table_entry new_symbol;
        new_symbol.location = section_table[current_section].size;
        section_table[current_section].size+= 4;//uvecavam je svesno
        table[addr->symbol] = new_symbol;

        int d = new_symbol.location - location_counter -4;
        instr = make_machine_instruction(opcode1, 15 ,gpr1, gpr2, d);
        reloc_location = new_symbol.location;
      }
      //create rellocation for symbol, because value is unknown
      create_relocation_entry(addr->symbol, reloc_location);
    }
  }
  data[current_section].write((char*)(&instr), sizeof(int));
}

bool Assembler::process_instruction_second_pass(yytokentype instr_token, string mnemonic, int line_n, Addressing *addr, int gpr1, int gpr2, int csr)
{
  switch(instr_token){
    case TOKEN_HALT:{
      //exporting instrukction to section_data
      int instr = make_machine_instruction(HALT);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_INT:{
      int instr = make_machine_instruction(INT);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_IRET:{
      //possibly wrong
      int instr = make_machine_instruction(IRET_MEM_STATUS, 0 , 14, 0, 4);
      data[current_section].write((char*)(&instr), sizeof(int));
      instr = make_machine_instruction(POP, 15 , 14, 0, 8);
      data[current_section].write((char*)(&instr), sizeof(int));
      location_counter+=4;
      break;
    }
    case TOKEN_RET:{
      //just pop pc
      int instr = make_machine_instruction(POP, 15 , 14, 0, 4);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_CALL:{
      make_bjmp_or_call(TOKEN_CALL, addr, 0, 0);
      break;
    }
    case TOKEN_JMP:{
      make_bjmp_or_call(TOKEN_JMP, addr, 0, 0);
      break;
    }
    case TOKEN_BEQ:{
      make_bjmp_or_call(TOKEN_BEQ, addr, gpr1, gpr2);
      break;
    }
    case TOKEN_BNE:{
      make_bjmp_or_call(TOKEN_BNE, addr, gpr1, gpr2);
      break;
    }
    case TOKEN_BGT:{
      make_bjmp_or_call(TOKEN_BGT, addr, gpr1, gpr2);
      break;
    }
    case TOKEN_PUSH:{
      int instr = make_machine_instruction(PUSH, 14, 0,gpr1, -4);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_POP:{
      //gprB je sp, gprA je gpr
      int instr = make_machine_instruction(POP, gpr1, 14,0, 4);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_XCHG:{ 
      int instr = make_machine_instruction(XCHG, 0, gpr1, gpr2, 0);
      data[current_section].write((char*)(&instr),sizeof(int));
      break;
    }
    case TOKEN_ADD:{
      int instr = make_machine_instruction(ADD, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_SUB:{
      int instr = make_machine_instruction(SUB, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_MUL:{
      int instr = make_machine_instruction(MUL, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_DIV:{
      int instr = make_machine_instruction(DIV, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_NOT:{
      int instr = make_machine_instruction(NOT, gpr1, gpr1, 0, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_AND:{
      int instr = make_machine_instruction(AND, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_OR:{
      int instr = make_machine_instruction(OR, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_XOR:{
      int instr = make_machine_instruction(XOR, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_SHL:{
      int instr = make_machine_instruction(SHL, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_SHR:{
      int instr = make_machine_instruction(SHR, gpr2, gpr2, gpr1, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_LD:{
      if(!make_ld_st(TOKEN_LD, addr, gpr1)){
        errors_to_print[line_n] = "Symbol value not found or value cannot be stored in 12 bits";
        return false;
      }else{
        if(addr->type == MEMDIR){
          location_counter+=4;
        }
      }

      break;
    }
    case TOKEN_ST:{
      if(!make_ld_st(TOKEN_ST, addr, gpr1)){
        errors_to_print[line_n] = "Symbol value not found or value cannot be stored in 12 bits";
        return false;
      } 
      break;
    }
    case TOKEN_CSRRD:{
      int instr = make_machine_instruction(CSRRD, gpr1, csr, 0, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
    case TOKEN_CSRWR:{
      int instr = make_machine_instruction(CSRWR, csr, gpr1, 0, 0);
      data[current_section].write((char*)(&instr), sizeof(int));
      break;
    }
  }
  location_counter+=4;
  return true;
}

void Assembler::print_symbol_table()
{
  txt_object_file << "#.symtab" << endl;
  txt_object_file << "Id\tValue\tTYPE\tBind\tNdx\tName" << endl;
  //sort symbol table by symbol id
  vector<pair<string,Symbol_table_entry>> mapVector(symbol_table.begin(),symbol_table.end());
  sort(mapVector.begin(), mapVector.end(), compareById);
  for(pair<string, Symbol_table_entry> iter: mapVector){
    txt_object_file << iter.second.id_temp << ":\t"; 
    txt_object_file << hex << setfill('0') << setw(4) << (0xffff & iter.second.value) << "\t";
    map<string,Section_table_entry>::iterator iter_sections = section_table.find(iter.first);
    if(iter_sections != section_table.end()){
      txt_object_file << "SCTN\t\t";
    }else{
      txt_object_file << "NOTYPE\t";
    }
    if(iter.second.global == false){
      txt_object_file << "LOC\t\t";
    }else{
      txt_object_file << "GLOB\t";
    }
    txt_object_file << dec;
    txt_object_file << section_table[iter.second.section].section_id << "\t\t";//id sekcije
    txt_object_file << iter.second.name << endl;
  }


}

void Assembler::print_section_table()
{
  txt_object_file << "#.sections" << endl;
  txt_object_file << "Id\tValue\tSize\tTYPE\tBind\tNdx\tName" << endl;
  for(map<string, Section_table_entry>::iterator it = section_table.begin(); it != section_table.end(); it++){

    txt_object_file << it->second.section_id << "\t"; 
    txt_object_file << hex << setfill('0') << setw(4) << (0xffff & 0) << "\t";
    txt_object_file << hex << setfill('0') << setw(4) << (0xffff & it->second.size) << "\t";
    txt_object_file << "SCTN\t";
    txt_object_file << "LOC\t\t\t";
    txt_object_file << dec;
    txt_object_file << it->second.section_id << "\t\t";
    txt_object_file << it->second.name << endl;
  }
}

void Assembler::print_reloc_table()
{
  for(map<string, vector<Reloc_table_entry>>::iterator it = relocation_table.begin(); it != relocation_table.end(); it++){
    txt_object_file << "#.reloc." << it->first << endl;
    txt_object_file << "Offset\tType\tSymbol\t\t\t\tAddend" << endl;
    vector <Reloc_table_entry> reloc_table_vector = it->second;
    for(Reloc_table_entry relocs : reloc_table_vector){
      txt_object_file << hex << setfill('0') << setw(4) << (0xffff & relocs.offset) << "\t" << relocs.type << "\t" << relocs.symbol << "\t" << setfill('0') << setw(4) << (0xffff & relocs.addend) << "\t" << endl;
    }
    txt_object_file << dec;
  }
}

void Assembler::print_sections_data()
{
 for(map<string, Section_table_entry>::iterator iter = section_table.begin(); iter != section_table.end();iter++){
  txt_object_file << "Section " << iter->first << ":"<<endl;
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
          
          data[entry.name].read(&c,sizeof(char));
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
    new_sym.section = "UND";
    new_sym.value = 0;
    symbol_table[symbol_name] = new_sym;
  }
  return true;
}

bool Assembler::process_section_first_pass(string section_name, int line_num)
{
  map<string, Section_table_entry>::iterator iter = section_table.find(section_name);
  if(iter != section_table.end()){
    errors_to_print[line_num] = "Section cannot be redeclared";
    return false;
  }
  if(current_section != ""){
    section_table[current_section].size = location_counter;
  }
  location_counter = 0;
  current_section = section_name;

  Symbol_table_entry new_symbol;
  new_symbol.defined = true;
  new_symbol.global = false;
  new_symbol.id_temp = current_id_symbol_table++;
  new_symbol.is_extern = false;
  new_symbol.name = section_name;
  new_symbol.section = section_name;
  new_symbol.value = 0;
  symbol_table[section_name] = new_symbol;
  //dodati za tabele literala

  Section_table_entry new_section;
  new_section.name = section_name;
  new_section.section_id = current_id_section_table++;
  new_section.size = 0;
  section_table[section_name] = new_section;

  data[section_name] = stringstream();
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
	fclose(myfile);
	// for(int i =0; i < list_of_lang_elems->size(); i++){
	// 	list_of_lang_elems->at(i)->print_le();
	// }

  if(first_pass() == false) return false;
  place_literal_pool();
  if(second_pass() == false) return false;
  write_literal_pool();
  create_binary_file();
  //create binary file
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
  return true;

}
Assembler *Assembler::instancePtr = nullptr;

void Assembler::print_errors()
{
  cout << "Assembler\n";
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
  location_counter+=text.size()+1;
  return true;
}

bool Assembler::process_instruction_first_pass(yytokentype instr_token, string mnemonic,int line_num,Addressing* addr)
{
  if(current_section == ""){
    errors_to_print[line_num] = "Invalid instruction " + mnemonic + " outside of a section!";
    return false;
  }
  else{
    switch(instr_token){
      //if an argument is literal, i have to pass it to a literal table
      case TOKEN_LD:{
        switch(addr->type){
          case MEMDIR:{
            location_counter+=4;
          }
          case IMMED:{
            if(addr->symbol != nullptr){
            //we have symbol
            //do nothing because symbol may not be defined
            }else{
              //literal
              int value = addr->literal;
              map<int, Literal_table_entry>& table = section_table[current_section].literal_table;
              map<int, Literal_table_entry>::iterator iter = table.find(value);
              if(iter != table.end()){
                //do nothing, literal already in table
              }else{
                Literal_table_entry new_literal;
                new_literal.location = 0;
                table[value] = new_literal;
              }
            }
          }
        }
        break;
      }
      case TOKEN_ST:{
        switch(addr->type){
          case MEMDIR:{
            if(addr->symbol != nullptr){
            //we have symbol
            //do nothing because symbol may not be defined
            }else{
              //literal
              int value = addr->literal;
              map<int, Literal_table_entry>& table = section_table[current_section].literal_table;
              map<int, Literal_table_entry>::iterator iter = table.find(value);
              if(iter != table.end()){
                //do nothing, literal already in table
              }else{
                Literal_table_entry new_literal;
                new_literal.location = 0;
                table[value] = new_literal;
              }
            }
            break;
          }
          //throw exception, fix this
          case IMMED:{
            errors_to_print[line_num] = "Store instruction used with absolute addressing";
            return false;
          }
        }
        break;
      }
      case TOKEN_CALL:{
      }
      case TOKEN_BEQ:{
      }
      case TOKEN_BNE:{
      }
      case TOKEN_BGT:{
      }
      case TOKEN_JMP:{
        //just one addressing type
        if(addr->symbol != nullptr){
          //we have symbol
          //do nothing because symbol may not be defined
        }else{
          //literal
          int value = addr->literal;
          map<int, Literal_table_entry>& table = section_table[current_section].literal_table;
          map<int, Literal_table_entry>::iterator iter = table.find(value);
          if(iter != table.end()){
            //do nothing, literal already in table
          }else{
            Literal_table_entry new_literal;
            new_literal.location = 0;
            table[value] = new_literal;
          }
        }
        break;
      }
      case TOKEN_IRET:{
        location_counter+=4;
        break;
      }
    }
    location_counter+=4;
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
  return true;
}

bool Assembler::process_skip_second_pass(int value)
{
  for(int i =0; i < value; i++){
    char c = 0;
    data[current_section].write((char*)(&c), sizeof(char));
  }
  location_counter += value;
  return true;
}

bool Assembler::process_word_second_pass(pair<bool, Uni> argument, int line_num)
{
  if(argument.first){
    int value = argument.second.num;
    data[current_section].write((char*)(&value), sizeof(int));
  }
  else{
    string symbol = argument.second.sym;
    create_relocation_entry(symbol);
    int c = 0;
    data[current_section].write((char*)(&c), sizeof(int));
    // }else{
    //   //symbol not found in symbol table
    //   errors_to_print[line_num] = "Directive word used undeclared symbol as argument";
    //   return false;
    // } 
  }
  location_counter += 4;
  return true;
}

bool Assembler::process_ascii_second_pass(string argument, int line_num)
{
  //just adding in current section data and adding to location_counter
  for(char c : argument){
    data[current_section].write((char*)(&c), sizeof(char));
  }
  char c = '\0';
  data[current_section].write((char*)(&c), sizeof(char));
  location_counter+=argument.size()+1;
  return true;
}

bool Assembler::process_section_second_pass(string section)
{
  current_section = section;//i just have to update this because i collected all data in the first pass
  location_counter = 0;
  return true;
}

