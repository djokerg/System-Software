#ifndef LANG_ELEM_HPP
#define LANG_ELEM_HPP

class Lang_Elem{
  public:
    Lang_Elem(int line_n){line_num = line_n;}
    virtual void print_le(){}
    virtual ~Lang_Elem(){}
  protected:
    int line_num;
};

#endif