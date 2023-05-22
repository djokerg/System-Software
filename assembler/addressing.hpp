#ifndef ADDRESSING_HPP
#define ADDRESSING_HPP
#include <iostream>
#include <string>
using namespace std;
class Addressing{
  string type;
  int literal;
  char* symbol;
  int gpr;
  char* csr;
  bool isCsr;
  public:
  Addressing(string type, int literal = -1, char* symbol = nullptr, int gpr = -1, char* csr = nullptr, bool isCsr = false){
    this->type = type;
    this->literal = literal;
    this->symbol = symbol;
    this->gpr = gpr;
    this->csr = csr;
    this->isCsr = isCsr;
  }

  void print_a(){
    cout << this->type << " ";
    if(literal != -1){
      cout << literal << " ";
    }
    if(gpr != -1){
      cout << gpr << " ";
    }
    cout << (symbol != nullptr? symbol: "");
    cout << (csr != nullptr? csr: "");
  }
};

#endif