#ifndef ADDRESSING_HPP
#define ADDRESSING_HPP
#include <iostream>
#include <string>
using namespace std;
class Addressing{
  string type;
  char* literal;
  char* symbol;
  char* reg;
  public:
  Addressing(string type, char* literal, char* symbol, char* reg){
    this->type = type;
    this->literal = literal;
    this->symbol = symbol;
    this->reg = reg;
  }

  void print_a(){
    cout << this->type << " ";
    cout << (literal != nullptr? literal: "");
    cout << (symbol != nullptr? symbol: "");
    cout << (reg != nullptr? reg: "");
  }
};

#endif