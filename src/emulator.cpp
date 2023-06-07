#include "../inc/emulator.hpp"
#include <termios.h>

Emulator * Emulator::instancePtr = nullptr;

size_t Emulator::MEMORY_SIZE = 1UL << 32;
unsigned int Emulator::START_PROGRAM_ADDRESS = 0x40000000;
int Emulator::NUM_OF_REGISTERS = 16;
unsigned int Emulator::MEMORY_MAPPED_REGISTERS = 0xFFFFFF00;
int Emulator::TIMER_STATUS_INDEX = 0;
int Emulator::TERMINAL_STATUS_INDEX = 1;
int Emulator::INTERUPT_STATUS_INDEX = 2;
int Emulator::FAULT_INSTRUCTION_CAUSE = 1;
int Emulator::TIMER_CAUSE = 2;
int Emulator::TERMINAL_CAUSE = 3;
int Emulator::SOFTWARE_CAUSE = 4;
unsigned int Emulator::TERM_IN = 0xFFFFFF04;
unsigned int Emulator::TERM_OUT = 0xFFFFFF00;
unsigned int Emulator::TIM_CFG = 0xFFFFFF10;

unsigned int Emulator::read_int_from_memory(unsigned int address)
{
  int* value = reinterpret_cast<int*>(static_cast<char*>(mapped_memory) + address);
  return *value;
}

char Emulator::read_byte_from_memory(unsigned int address)
{
  char* c = static_cast<char*>(static_cast<char*>(mapped_memory) + address);
  return *c;
}
//ovo address moze biti i int
void Emulator::write_int_to_memory(unsigned int address, int num)
{
  if(address == TERM_OUT){
    cout << (char)num << flush;
  }
  int* value = reinterpret_cast<int*>(static_cast<char*>(mapped_memory) + address);
  *value = num;
}

void Emulator::write_byte_to_memory(unsigned int address, char c)
{
  if(address == TERM_OUT){
    cout << c;
  }
  char* c_mem = static_cast<char*>(static_cast<char*>(mapped_memory) + address);
  *c_mem = c;
}

void Emulator::set_flag(int flag)
{
  cs_registers[status] |= flag;
}

int Emulator::get_flag(int flag)
{
  return cs_registers[status] & flag;
}

void Emulator::reset_flag(int flag)
{
  cs_registers[status] &= ~flag;
}

Emulator::Emulator() : mem_dump("mem_dump.txt"),is_running(false),gp_registers(NUM_OF_REGISTERS, 0),cs_registers(3, 0), terminal_interrupt(false){}

bool Emulator::create_segment_table()
{
  ifstream input_file(executable_file, ios::binary);
  if(!input_file){
    errors_to_print.push_back("File " + executable_file + " does not exist");
    return false;
  }
  int current_segment_id = 0;
  int num_of_seg = 0;
  input_file.read((char *)&num_of_seg, sizeof(num_of_seg));
  for(int i = 0; i < num_of_seg; i++){
    Segment seg;
    input_file.read((char *)(&seg.address), sizeof(seg.address));
    //now read segment size
    input_file.read((char *)(&seg.size), sizeof(seg.size));
    segment_table[current_segment_id++] = seg;
    //now read all segment data
    stringstream* new_seg_data = new stringstream();
    string data;
    data.resize(seg.size);
    input_file.read((char*)data.c_str(), seg.size);
    new_seg_data->write(data.c_str(), data.size());
    segment_data[current_segment_id - 1] = new_seg_data;//added all
  }
  
  return true;
}

void Emulator::print_segment_table()
{
  mem_dump << "Loaded segments from hex file" << endl;
  for(map<int, Segment>::iterator iter = segment_table.begin();iter!=segment_table.end();iter++){
    mem_dump << "Segment: " << iter->first << endl;
    mem_dump << "Address: " << hex << iter->second.address << endl;
    mem_dump << "Size: " << iter->second.size << endl;
    int rows = (iter->second.size-1)/8+1;
    for(int i = 0; i < rows;i++){
      mem_dump << hex << setfill('0') << setw(4) << (0xffff & i*8) << ": ";
      for(int j = i*8; j < (i+1)*8;j++){
        char c;
        if(j<iter->second.size){
          segment_data[iter->first]->read(&c,sizeof(char));
        }
        else{
          c = 0;
        }
          mem_dump << hex << setfill('0') << setw(2) << (0xff & c) << " ";
          mem_dump << dec;
      } 
      mem_dump << endl;
    }
  }
}

bool Emulator::load_segment_data()
{
  //creating mapped memory with mmap function
  //check if this is right, add 4 this like blocks
  mapped_memory = mmap(nullptr, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE, -1, 0);
  if(mapped_memory == MAP_FAILED){
    errors_to_print.push_back("Error during memory mapping");
    return false;
  }
  //sort segments by address, they will come one by other, so i can write them to file 
  vector<pair<int, Segment>> sortedVector(segment_table.begin(), segment_table.end());
  sort(sortedVector.begin(), sortedVector.end(), compareByValue);
  for(pair<int, Segment> segment : sortedVector){
    //map this in new file
    char* target = static_cast<char*>(mapped_memory) + segment.second.address;
    memcpy(target, segment_data[segment.first]->str().c_str(), segment.second.size);
  }
  return true;
}

bool Emulator::compareByValue(const pair<int, Segment> &a, const pair<int, Segment> &b)
{
  return a.second.address < b.second.address;
}

void Emulator::mem_dump_fn()
{
  
  vector<pair<int, Segment>> sortedVector(segment_table.begin(), segment_table.end());
  sort(sortedVector.begin(), sortedVector.end(), compareByValue);
  mem_dump << "-------------------------------------------------" << endl;
  mem_dump << "Memory mapped segments at the end of the program" << endl;
  for(pair<int,Segment> s: sortedVector){
    unsigned int first = s.second.address;
    int rows = (s.second.size-1)/8+1;
    for(unsigned int i = 0; i < rows;i++){
      mem_dump << hex << setfill('0') << setw(8) << (0xffffffff & (i*8+s.second.address)) << ": ";
      for(int j = i*8; j < (i+1)*8;j++){
        char c;
        if(j<s.second.size){
          c = read_byte_from_memory(j+first);
        }
        else{
          c = 0;
        }
          mem_dump << hex << setfill('0') << setw(2) << (0xff & c) << " ";
          mem_dump << dec;
      }
      mem_dump << endl;
    }
    mem_dump << dec;
  }
}

bool Emulator::start_program()
{
  rpc = START_PROGRAM_ADDRESS;
  //sp shows at the top of accesible address space
  rsp = MEMORY_MAPPED_REGISTERS;
  //instrution for memory read
  reset_flag(Tl);
  reset_flag(Tr);
  reset_flag(I);

  this->is_running = true;
  config_terminal();

  while(is_running){

    if(!instruction_fetch_and_execute()){
      //call interupt
      generate_interrupt(FAULT_INSTRUCTION_CAUSE);
      reset_terminal();
      return false;
    }
    //check for terminal interrupt
    read_from_stdin_to_term_in();

    if(terminal_interrupt){
      generate_interrupt(TERMINAL_CAUSE);
      terminal_interrupt = false;
    }
  }
  reset_terminal();
  return true;
}

void Emulator::generate_interrupt(int cause_param)
{
  //now i have to put cause in cause register, push status i push pc
  push(cs_registers[status]);
  push(rpc);
  //change rpc, so it points to handler
  cs_registers[cause] = cause_param;
  rpc = cs_registers[handler];//now i will jump to interrupt

  set_flag(I);
  set_flag(Tl);
  set_flag(Tr);

}

void Emulator::push(int val)
{
  //first decrement, than write to that location
  rsp-=4;
  write_int_to_memory(rsp,val);
}

int Emulator::pop()
{
  int val = read_int_from_memory(rsp);
  rsp+=4;
  return val;
}

bool Emulator::instruction_fetch_and_execute()
{
  unsigned int full_instr = read_int_from_memory(rpc);
  //as soon as i read instruction, i increase pc
  rpc+=4;
  int byte1 = full_instr>>24;
  int byte2 = full_instr>>16 & 0xff;
  int byte34 = full_instr & 0xffff;
  mnemonic = (OP_CODE_MOD)byte1;
  regA = byte2>>4;
  regB = byte2 & 0xf;
  regC = byte34>>12;
  disp = byte34 & 0x0fff;
  if(disp & 0x800){
    disp = -((~disp & 0xfff)+ 1);
  }
  if(regA > 15 || regA < 0 || regB > 15 || regB < 0 || regC > 15 || regC < 0){
    //non valid register number
    errors_to_print.push_back("Register number fault");
    return false;
  }
  //fetch phase over, now i need to see if this instruction exists, and throw an error if its not
  //check if this instruction even exists - if it doesnt exist, then 
  switch(mnemonic){
    case HALT: {
      is_running = false;
      break;
    }
    case INT:{
      push(cs_registers[status]);
      push(rpc);
      //change rpc, so it points to handler
      cs_registers[cause] = SOFTWARE_CAUSE;
      cs_registers[status] = cs_registers[status] &(~0x1);
      rpc = cs_registers[handler];
      break;
    }
    case XCHG:{
      int tmp = gp_registers[regB];
      gp_registers[regB] = gp_registers[regC];
      gp_registers[regC] = tmp;
      break;
    }
    case ADD:{
      gp_registers[regA] = gp_registers[regB] + gp_registers[regC];
      break;
    }
    case SUB:{
      gp_registers[regA] = gp_registers[regB] - gp_registers[regC];
      break;
    }
    case MUL:{
      gp_registers[regA] = gp_registers[regB] * gp_registers[regC];
      break;
    }
    case DIV:{
      gp_registers[regA] = gp_registers[regB] / gp_registers[regC];
      break;
    }
    case NOT:{
      gp_registers[regA] = ~gp_registers[regB];
      break;
    }
    case AND:{
      gp_registers[regA] = gp_registers[regB] & gp_registers[regC];
      break;
    }
    case OR:{
      gp_registers[regA] = gp_registers[regB] | gp_registers[regC];
      break;
    }
    case XOR:{
      gp_registers[regA] = gp_registers[regB] ^ gp_registers[regC];
      break;
    }
    case SHL:{
      gp_registers[regA] = gp_registers[regB] << gp_registers[regC];
      break;
    }
    case SHR:{
      gp_registers[regA] = gp_registers[regB] >> gp_registers[regC];
      break;
    }
    case CSRRD:{
      gp_registers[regA] = cs_registers[regB];
      break;
    }
    case CSRWR:{
      cs_registers[regA] = gp_registers[regB];
      break;
    }
    case PUSH:{
      gp_registers[regA] = gp_registers[regA] + disp;
      write_int_to_memory(gp_registers[regA],gp_registers[regC]);
      break;
    }
    case POP:{
      gp_registers[regA] = read_int_from_memory(gp_registers[regB]);
      gp_registers[regB] = gp_registers[regB] + disp;
      break;
    }
    case JMP_MEM:{
      rpc = read_int_from_memory(gp_registers[regA]+disp);
      break;
    }
    case JMP:{
      rpc = gp_registers[regA] + disp;
      break;
    }
    case BEQ_MEM:{
      if(gp_registers[regB] == gp_registers[regC]){
        rpc = read_int_from_memory(gp_registers[regA]+disp);
      }
      break;
    }
    case BEQ:{
      if(gp_registers[regB] == gp_registers[regC]){
        rpc = gp_registers[regA] + disp;
      }
      break;
    }
    case BNE_MEM:{
      if(gp_registers[regB] != gp_registers[regC]){
        rpc = read_int_from_memory(gp_registers[regA]+disp);
      }
      break;
    }
    case BNE:{
      if(gp_registers[regB] != gp_registers[regC]){
        rpc = gp_registers[regA] + disp;
      }
      break;
    }
    case BGT_MEM:{
      if(gp_registers[regB] > gp_registers[regC]){
        rpc = read_int_from_memory(gp_registers[regA]+disp);
      }
      break;
    }
    case BGT:{
      if(gp_registers[regB] > gp_registers[regC]){
        rpc = gp_registers[regA] + disp;
      }
      break;
    }
    case CALL_MEM:{
      push(rpc);
      rpc = read_int_from_memory(gp_registers[regA] + gp_registers[regB] + disp);
      break;
    }
    case CALL:{
      push(rpc);
      rpc = gp_registers[regA] + gp_registers[regB] + disp;
      break;
    }
    case LD_MEM:{
      gp_registers[regA] = read_int_from_memory(gp_registers[regB] + gp_registers[regC] + disp);
      break;
    }
    case LD_GPR:{
      gp_registers[regA] = gp_registers[regB] + disp;
      break;
    }
    case ST_MEM:{
      write_int_to_memory(gp_registers[regA] + gp_registers[regB] + disp,gp_registers[regC]);
      break;
    }
    case ST_MEM_MEM:{
      write_int_to_memory(read_int_from_memory(gp_registers[regA] + gp_registers[regB] + disp),gp_registers[regC]);
      break;
    }
    case CSR_WR_MEM:{
      cs_registers[regA] = read_int_from_memory(gp_registers[regB] + gp_registers[regC] + disp);
      break;
    }
    case CSR_WR_CSR:{
      cs_registers[regA] = cs_registers[regB] | disp;
      break; 
    }
    case CSR_POP:{
      cs_registers[regA] = read_int_from_memory(gp_registers[regB]);
      gp_registers[regB] = gp_registers[regB] + disp;
      break;
    }
    default:{
      errors_to_print.push_back("Instruction doesn't exist or it's not reachable from assembler");//can be updated
      return false;
    }
  }
  return true;
}

void Emulator::print_all_registers()
{
  cout << endl << "-----------------------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state:" << endl;
  for(int i = 0; i <= 15;i++){
    if(i>0 && i%4 == 0){
      cout << endl;
    }
    cout << "\tr" << dec << i << ":0x" << hex << setfill('0') << setw(8) << gp_registers[i];
  }
}

struct termios backup_settings, new_settings;

void Emulator::config_terminal()
{
  tcgetattr(STDIN_FILENO, &backup_settings);

  new_settings = backup_settings;

  new_settings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);

  //set waiting time at 0

  new_settings.c_cc[VTIME] = 0;
  new_settings.c_cc[VMIN] = 0;

  //set new settings to terminal
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_settings);

}
void Emulator::reset_terminal()
{
  //reset backup settings to STDIN file terminal
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &backup_settings);
}
void Emulator::read_from_stdin_to_term_in()
{
  //if there are characters to
  char ch;
  if(read(STDIN_FILENO, &ch, 1) == 1){
    write_int_to_memory(TERM_IN,(int)ch);
    terminal_interrupt = true;
  }
}
bool Emulator::execute_file()
{
  if(!create_segment_table()){
    return false;
  }
  print_segment_table();
  if(!load_segment_data()){
    return false;
  }
  //mem_dump();-dont print like this, bad way
  //all needed data is loaded, now i can start reading from memory and executing
  if(!start_program()){
    return false;
  }
  mem_dump_fn();
  print_all_registers();
  mem_dump.close();
  munmap(mapped_memory,MEMORY_SIZE);
  return true;
}

void Emulator::print_errors()
{
  cout << "Emulator\n";
  cout << "Error messages:" << "\n";
  for(string error : errors_to_print){
    cout << error << endl;
  }
}
