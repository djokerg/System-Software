#include "instruction.hpp"
#include "assembler.hpp"

bool Instruction::visit_first_pass()
{
  return Assembler::getInstance()->process_instruction_first_pass(mnemonic, line_num);
}

bool Instruction::visit_second_pass()
{
  return true;
}
