#ifndef ADDRESSING_HPP
#define ADDRESSING_HPP
#include <iostream>
#include <string>
using namespace std;
class Assembler;

enum Addr_Type{
  IMMED = 0,
  MEMDIR,
  REGDIR,
  REGIND,
  REGINDPOM
};

class Addressing{
  friend class Assembler;
  Addr_Type type;
  int literal;
  char* symbol;
  int gpr;
  int csr;
  bool isCsr;
  public:
  Addressing(Addr_Type type, int literal = -1, char* symbol = nullptr, int gpr = -1, int csr = -1, bool isCsr = false){
    this->type = type;
    this->literal = literal;
    this->symbol = symbol;
    this->gpr = gpr;
    this->csr = csr;
    this->isCsr = isCsr;
  }
  //delete this
  void print_a(){
    if(literal != -1){
      cout << literal << " ";
    }
    if(gpr != -1){
      cout << gpr << " ";
    }
    cout << (symbol != nullptr? symbol: "");
    if(csr != -1){
      cout << csr << " ";
    }
  }
};

#endif