#ifndef DIRECTIVE_HPP
#define DIRECTIVE_HPP

#include <string>
#include "lang_elem.hpp"
#include <vector>
#include <iostream>
#include <map>
#include "parser.hpp"

using namespace std;
union Uni{
  int num;
  char* sym;
};

class Directive: public Lang_Elem {
  vector<pair<bool, Uni>>* arg_list;
  string mnemonic;
  yytokentype token_type;
  public:

    Directive(yytokentype token_type, int line_n, string mnemonic, vector<pair<bool, Uni>>* arg_list = nullptr):Lang_Elem(line_n){
      this->mnemonic = mnemonic;
      this->arg_list = arg_list;
      this->token_type = token_type;
    }

    void print_directive(){
      cout << this->line_num << " ";
      cout << mnemonic << " ";
      if(arg_list != NULL){
      for(int i = 0; i < arg_list->size();i++){
        if(i != arg_list->size()-1){
          if(arg_list->at(i).first){
            cout << arg_list->at(i).second.num << ",";
          }else{
            cout << arg_list->at(i).second.sym << ",";
          }
        }else{
          if(arg_list->at(i).first){
            cout << arg_list->at(i).second.num;
          }else{
            cout << arg_list->at(i).second.sym;
          }
        }
      }
      }
      cout << "\n";
    }

    void print_le(){
      print_directive();
    }

    bool visit_first_pass() override;

    bool visit_second_pass() override;


    ~Directive(){
      free(arg_list);
    }
};

#endif