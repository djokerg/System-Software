#include "lang_elem.hpp"
#include "addressing.hpp"
#include <iostream>
#include <string>
using namespace std;
class Instruction: public Lang_Elem{
  char* mnemonic;
  Addressing* addressing;
  int gpr1;
  int gpr2;
  char* csr;
  public:
  Instruction(int line_n, char* mnemonic, Addressing* addr = nullptr, int gpr1 = -1, int gpr2 = -1, char* csr = nullptr):Lang_Elem(line_n){
    this->mnemonic = mnemonic;
    this->addressing = addr;
    this->gpr1 = gpr1;
    this->gpr2 = gpr2;
    this->csr = csr;
  }

  void print_instruction(){
    if(mnemonic != NULL){
      cout << line_num << " ";
      cout << mnemonic << " ";
      if(gpr1 != -1){
        cout << gpr1 << " ";
      }
      if(gpr2 != -1){
        cout << gpr2 << " ";
      }
      cout << (csr!= nullptr? csr: "");
    }
  }
  void print_le(){
    print_instruction();
    if(this->addressing) this->addressing->print_a();
    cout << "\n";
  }
};