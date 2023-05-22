#include <string>
#include "lang_elem.hpp"
#include <vector>
#include <iostream>
#include <map>
using namespace std;

union Uni{
  int num;
  char* sym;
};

class Directive: public Lang_Elem {
  vector<pair<bool, Uni>>* arg_list;
  string mnemonic;
  public:
    Directive(int line_n, string mnemonic, vector<pair<bool, Uni>>* arg_list = nullptr):Lang_Elem(line_n){
      this->mnemonic = mnemonic;
      this->arg_list = arg_list;
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

    ~Directive(){
      free(arg_list);
    }
};