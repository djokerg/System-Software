#include <string>
#include "lang_elem.hpp"
#include <vector>
#include <iostream>
using namespace std;

class Directive: public Lang_Elem {
  string identificator;
  vector<string>* arg_list;
  public:
    Directive(int line_n, string id, vector<string>* arg_list):Lang_Elem(line_n){
      this->identificator = id;
      this->arg_list = arg_list;
    }

    void print_directive(){
      cout << this->line_num;
      cout << identificator;
      for(int i = 0; i < arg_list->size();i++){
        cout << arg_list->at(i);
      }
    }
    ~Directive(){
      free(arg_list);
    }
};