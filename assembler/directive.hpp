#include <string>
#include "lang_elem.hpp"
#include <vector>
#include <iostream>
using namespace std;

class Directive: public Lang_Elem {
  string mnemonic;
  vector<string>* arg_list;
  public:
    Directive(int line_n, string id, vector<string>* arg_list):Lang_Elem(line_n){
      this->mnemonic = id;
      this->arg_list = arg_list;
    }

    

    void print_directive(){
      cout << this->line_num;
      cout << mnemonic << " ";
      if(arg_list != NULL){
      for(int i = 0; i < arg_list->size();i++){
        if(i != arg_list->size()-1){
        cout << arg_list->at(i) << ",";
        }else{
          cout << arg_list->at(i) << "\n";
        }
      }
      }
    }

    void print_le(){
      print_directive();
    }

    ~Directive(){
      free(arg_list);
    }
};