#include "../inc/emulator.hpp"

int main(int argc, const char* argv[]){
  if(argc >2){
    cout << "Only one file can be executed at once";
    return -1;
  }
  Emulator* emul = Emulator::getInstance();
  emul->initialize(argv[1]);
  if(!emul->execute_file()){
    emul->print_errors();
  }
  return 0;
}