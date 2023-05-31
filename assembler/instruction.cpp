#include "instruction.hpp"
#include "assembler.hpp"

bool Instruction::visit_first_pass()
{
  //in case of branch or jmp instruction adress field is worthless
  return Assembler::getInstance()->process_instruction_first_pass(token_type, mnemonic, line_num, addressing);
}

bool Instruction::visit_second_pass()
{
  return Assembler::getInstance()->process_instruction_second_pass(token_type,mnemonic, line_num, addressing, gpr1, gpr2, csr);
}
