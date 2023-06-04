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
  enum Instruction{
    HALT,
    INT,
    IRET,
    CALL,
    RET,
    JMP,
    BEQ,
    BNE,
    BGT,
    PUSH,
    POP,
    XCHG,
    ADD,
    SUB,
    MUL,
    DIV,
    NOT,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
    LD,
    ST,
    CSRRD,
    CSRWR
  };

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
  enum Addressing{
    imm,
    memdir,
    regdir,
    regind,
    regindpom
  };

  void* mapped_memory;
  vector<int> gp_registers;
  vector<int> cs_registers;
  //for easy accesss
  int& rpc = gp_registers[pc];
  int& rsp = gp_registers[sp];

  bool is_running;

  //current instruction data
  Instruction mnemonic;
  int regA;
  int regB;
  int regC;
  int disp;
  Addressing addr_type;

  static Emulator* instancePtr;
  Emulator();
  string executable_file;
  vector<string> errors_to_print;
  ofstream emulator_debugging_file;
  struct Segment{
    int address;
    int size;
  };

  map<int, Segment> segment_table;
  map<int, stringstream*> segment_data;

  bool create_segment_table();
  void print_segment_table();
  bool load_segment_data();
  static bool compareByValue(const pair<int, Segment>& a, const pair<int, Segment>& b);
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
  static int START_PROGRAM_ADDRESS;
  static int NUM_OF_REGISTERS;
  static int MEMORY_MAPPED_REGISTERS;
  static int FAULT_INSTRUCTION_CAUSE;
  static int TIMER_CAUSE;
  static int TERMINAL_CAUSE;
  static int SOFTWARE_CAUSE;
  static int TERM_OUT;
  static int TERM_IN;
  static int TIM_CFG;
  static int TIMER_STATUS_INDEX;
  static int TERMINAL_STATUS_INDEX;
  static int INTERUPT_STATUS_INDEX;
  
};