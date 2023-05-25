#ifndef LANG_ELEM_HPP
#define LANG_ELEM_HPP
#include <iostream>
using namespace std;

class Lang_Elem{
  public:
    Lang_Elem(int line_n){line_num = line_n;}
    virtual void print_le(){}
    virtual bool visit_first_pass() = 0;
    virtual bool visit_second_pass() = 0;
    virtual ~Lang_Elem(){}
  protected:
    int line_num;
    
};

#endif