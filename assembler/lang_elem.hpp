
class Lang_Elem{
  public:
    Lang_Elem(int line_n){line_num = line_n;}
    virtual ~Lang_Elem(){}
  protected:
    int line_num;
};