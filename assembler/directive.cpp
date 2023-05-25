#include "directive.hpp"
#include "assembler.hpp"

bool Directive::visit_first_pass()
{
  switch(token_type){
    case TOKEN_LABEL:{
      return Assembler::getInstance()->process_label_first_pass(this->mnemonic, line_num);
    }
    case TOKEN_GLOBAL:{
      for(int i = 0; i < arg_list->size();i++){
        Assembler::getInstance()->process_global_first_pass(arg_list->at(i).second.sym, line_num);
      }
      return true;
    }
    case TOKEN_EXTERN:{
      for(int i = 0; i < arg_list->size();i++){
        Assembler::getInstance()->process_extern_first_pass(arg_list->at(i).second.sym, line_num);
      }
      return true;
    }
    case TOKEN_SECTION:{      
      return Assembler::getInstance()->process_section_first_pass(arg_list->at(0).second.sym, line_num);
    }
    case TOKEN_ASCII:{
      string string_arg = arg_list->at(0).second.sym+1;
      string_arg.pop_back();
      return Assembler::getInstance()->process_ascii_first_pass(string_arg, line_num);
    }
    case TOKEN_WORD:{
      for(int i =0; i < arg_list->size();i++){
        Assembler::getInstance()->process_word_first_pass(arg_list->at(i), line_num);
      }
      return true;
    }
    case TOKEN_SKIP:{
      return Assembler::getInstance()->process_skip_first_pass(arg_list->at(0).second.num, line_num);
    }
    case TOKEN_END:{
      return Assembler::getInstance()->process_end_directive(line_num);
    }
  }
  return true;
}

bool Directive::visit_second_pass()
{
  
  return true;
}
