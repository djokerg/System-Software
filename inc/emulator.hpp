#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <sys/mman.h>
#include <iomanip>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
using namespace std;

class Emulator{
  //all structures that emulator needs to emulate this pc
  //bunch of enums for easier access
  enum OP_CODE_MOD{
    HALT = 0b00000000,
    INT = 0b00010000,
    XCHG = 0b01000000,
    ADD = 0b01010000,
    SUB = 0b01010001,
    MUL = 0b01010010,
    DIV = 0b01010011,
    NOT = 0b01100000,
    AND = 0b01100001,
    OR = 0b01100010,
    XOR = 0b01100011,
    SHL = 0b01110000,
    SHR = 0b01110001,
    CSRRD = 0b10010000,
    CSRWR = 0b10010100,
    PUSH = 0b10000001,
    POP = 0b10010011,
    JMP_MEM = 0b00111000,
    JMP = 0b00110000,
    BEQ_MEM = 0b00111001,
    BEQ = 0b00110001,
    BNE_MEM = 0b00111010,
    BNE = 0b00110010,
    BGT_MEM = 0b00111011,
    BGT = 0b00110011,
    CALL_MEM = 0b00100001,
    CALL = 0b00100000,
    LD_MEM = 0b10010010,
    LD_GPR = 0b10010001,
    ST_MEM = 0b10000000,
    ST_MEM_MEM = 0b10000010,
    CSR_WR_MEM = 0b10010110,
    CSR_WR_CSR = 0b10010101,
    CSR_POP = 0b10010111
  };  
  //csr_wr_mem used in iret instruction
  enum GPRegisters{
    r0 = 0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
    pc = 15,
    sp = 14
  };
  enum CSRegisters{
    status = 0,
    handler,
    cause
  };
  enum StatusFlag{
    Tr = 1,
    Tl = 1 << 1,
    I = 1 << 2
  };

  void* mapped_memory;

  unsigned int read_int_from_memory(unsigned int address);
  char read_byte_from_memory(unsigned int address);
  void write_int_to_memory(unsigned int address, int num);
  void write_byte_to_memory(unsigned int address, char c);
  
  vector<long> gp_registers;
  vector<long> cs_registers;

  void set_flag(int flag);
  int get_flag(int flag);
  void reset_flag(int flag);
  //for easy accesss
  long& rpc = gp_registers[pc];
  long& rsp = gp_registers[sp];

  bool is_running;
  bool terminal_interrupt;

  //current instruction data
  OP_CODE_MOD mnemonic;
  int regA;
  int regB;
  int regC;
  int disp;

  static Emulator* instancePtr;
  Emulator();
  string executable_file;
  vector<string> errors_to_print;
  ofstream mem_dump;
  struct Segment{
    unsigned int address;
    int size;
  };

  map<int, Segment> segment_table;
  map<int, stringstream*> segment_data;

  bool create_segment_table();
  void print_segment_table();
  bool load_segment_data();
  static bool compareByValue(const pair<int, Segment>& a, const pair<int, Segment>& b);
  void mem_dump_fn();
  bool start_program();
  void generate_interrupt(int cause);
  void push(int val);
  int pop();
  bool instruction_fetch_and_execute();
  void print_all_registers();

  void config_terminal();
  void reset_terminal();
  void read_from_stdin_to_term_in();
public:
  Emulator(const Emulator& obj) = delete;
  
  static Emulator* getInstance(){
    if(instancePtr == nullptr){
      instancePtr = new Emulator();
    }
    return instancePtr;
  }

  void initialize(string file_name){
    executable_file = file_name;
  }

  bool execute_file();

  void print_errors();

  //all constants i need
  static size_t MEMORY_SIZE;
  static unsigned int START_PROGRAM_ADDRESS;
  static int NUM_OF_REGISTERS;
  static unsigned int MEMORY_MAPPED_REGISTERS;
  static int FAULT_INSTRUCTION_CAUSE;
  static int TIMER_CAUSE;
  static int TERMINAL_CAUSE;
  static int SOFTWARE_CAUSE;
  static unsigned int TERM_OUT;
  static unsigned int TERM_IN;
  static unsigned int TIM_CFG;
  static int TIMER_STATUS_INDEX;
  static int TERMINAL_STATUS_INDEX;
  static int INTERUPT_STATUS_INDEX;
  
};