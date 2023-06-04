#include "emulator.hpp"

Emulator * Emulator::instancePtr = nullptr;

size_t Emulator::MEMORY_SIZE = 1UL << 32;
int Emulator::START_PROGRAM_ADDRESS = 0x40000000;
int Emulator::NUM_OF_REGISTERS = 16;
int Emulator::MEMORY_MAPPED_REGISTERS = 0xFFFFFF00;
int Emulator::TIMER_STATUS_INDEX = 0;
int Emulator::TERMINAL_STATUS_INDEX = 1;
int Emulator::INTERUPT_STATUS_INDEX = 2;
int Emulator::FAULT_INSTRUCTION_CAUSE = 1;
int Emulator::TIMER_CAUSE = 2;
int Emulator::TERMINAL_CAUSE = 3;
int Emulator::SOFTWARE_CAUSE = 4;
int Emulator::TERM_IN = 0xFFFFFF04;
int Emulator::TERM_OUT = 0xFFFFFF00;
int Emulator::TIM_CFG = 0xFFFFFF10;

Emulator::Emulator():emulator_debugging_file("emulator_debugging_file.txt"){}

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
  for(map<int, Segment>::iterator iter = segment_table.begin();iter!=segment_table.end();iter++){
    emulator_debugging_file << "Segment: " << iter->first << endl;
    emulator_debugging_file << "Address: " << iter->second.address << endl;
    emulator_debugging_file << "Size: " << iter->second.size << endl;
    int rows = (iter->second.size-1)/8+1;
    for(int i = 0; i < rows;i++){
      emulator_debugging_file << hex << setfill('0') << setw(4) << (0xffff & i*8) << ": ";
      for(int j = i*8; j < (i+1)*8;j++){
        char c;
        if(j<iter->second.size){
          segment_data[iter->first]->read(&c,sizeof(char));
        }
        else{
          c = 0;
        }
          emulator_debugging_file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
          emulator_debugging_file << dec;
      } 
      emulator_debugging_file << endl;
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
    char* mappedData = (char*)mapped_memory;
    const char* stringStrData = segment_data[segment.first]->str().data();
    memcpy(mappedData + segment.second.address, stringStrData, segment.second.size);
  }
  return true;
}

bool Emulator::compareByValue(const pair<int, Segment> &a, const pair<int, Segment> &b)
{
  return a.second.address < b.second.address;
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
  return true;
}

void Emulator::print_errors()
{
  for(string error : errors_to_print){
    emulator_debugging_file << error << endl;
  }
}
