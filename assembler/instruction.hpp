#include "lang_elem.hpp"
#include "addressing.hpp"
#include <iostream>
using namespace std;
class Instruction: public Lang_Elem{
  char* mnemonic;
  Addressing* addressing;
  char* gpr1;
  char* gpr2;
  public:
  Instruction(int line_n, char* mnemonic, Addressing* addr, char* gpr1, char* gpr2):Lang_Elem(line_n){
    this->mnemonic = mnemonic;
    this->addressing = addr;
    this->gpr1 = gpr1;
    this->gpr2 = gpr2;
  }

  void print_instruction(){
    if(mnemonic != NULL){
      cout << line_num << " ";
      cout << mnemonic << " ";
      gpr1!=NULL? cout << gpr1: cout << "" ;
      cout << " ";
      gpr2!=NULL? cout << gpr2: cout << "" ;
      cout << " ";
    }
  }
  void print_le(){
    print_instruction();
    if(this->addressing) this->addressing->print_a();
    cout << "\n";
  }
};