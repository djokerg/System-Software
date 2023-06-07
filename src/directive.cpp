#include "../inc/directive.hpp"
#include "../inc/assembler.hpp"

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
      string new_string;
      int cnt = 0;
      while(cnt < string_arg.size()){
        if(string_arg.at(cnt) == '\\' && cnt < string_arg.size()-1){
          switch(string_arg.at(cnt+1)){
            case 'n': new_string.push_back('\n'); break;
            case 't': new_string.push_back('\t'); break;
            case 'r': new_string.push_back('\r'); break;
            case 'f': new_string.push_back('\f'); break;
            case 'v': new_string.push_back('\v'); break;
          }
          cnt= cnt+2;
        }else{
          new_string.push_back(string_arg.at(cnt));
          cnt++;
        }
      }
      return Assembler::getInstance()->process_ascii_first_pass(new_string, line_num);
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
  switch(token_type){
    case TOKEN_SKIP:{
      return Assembler::getInstance()->process_skip_second_pass(arg_list->at(0).second.num);
    }
    case TOKEN_WORD:{
      for(int i =0; i < arg_list->size();i++){
          Assembler::getInstance()->process_word_second_pass(arg_list->at(i), line_num);
        }
      return true;
    }
    case TOKEN_ASCII:{
      string string_arg = arg_list->at(0).second.sym+1;
      string_arg.pop_back();
      string new_string;
      int cnt = 0;
      while(cnt < string_arg.size()){
        if(string_arg.at(cnt) == '\\' && cnt < string_arg.size()-1){
          switch(string_arg.at(cnt+1)){
            case 'n': new_string.push_back('\n'); break;
            case 't': new_string.push_back('\t'); break;
            case 'r': new_string.push_back('\r'); break;
            case 'f': new_string.push_back('\f'); break;
            case 'v': new_string.push_back('\v'); break;
          }
          cnt= cnt+2;
        }else{
          new_string.push_back(string_arg.at(cnt));
          cnt++;
        }
      }
      return Assembler::getInstance()->process_ascii_second_pass(new_string, line_num);
    }
    case TOKEN_SECTION:{
      return Assembler::getInstance()->process_section_second_pass(arg_list->at(0).second.sym);
    }
    case TOKEN_GLOBAL:{
      bool flag = true;
      for(int i = 0; i < arg_list->size();i++){
        if(flag){
          flag = Assembler::getInstance()->process_global_second_pass(arg_list->at(i).second.sym, line_num);
        }else{
          return flag;
        }
      }
      return flag;
    }
  }
  return true;
}
