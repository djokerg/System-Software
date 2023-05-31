#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include "lang_elem.hpp"
#include "addressing.hpp"
#include <iostream>
#include <string>
using namespace std;
#include "parser.hpp"

class Instruction: public Lang_Elem{
  char* mnemonic;
  Addressing* addressing;
  int gpr1;
  int gpr2;
  int csr;
  yytokentype token_type;

  public:
  Instruction(yytokentype token_type, int line_n, char* mnemonic, Addressing* addr = nullptr, int gpr1 = -1, int gpr2 = -1, int csr = -1):Lang_Elem(line_n){
    this->mnemonic = mnemonic;
    this->addressing = addr;
    this->gpr1 = gpr1;
    this->gpr2 = gpr2;
    this->csr = csr;
    this->token_type = token_type;
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
      if(csr != -1){
        cout << csr << " ";
      }
    }
  }

  bool visit_first_pass() override;

  bool visit_second_pass() override;
  
  void print_le(){
    print_instruction();
    if(this->addressing) this->addressing->print_a();
    cout << "\n";
  }
};

#endif