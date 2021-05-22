#include "z80_disassembly.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "z80.h"
#include "cap32.h"
#include "log.h"

extern t_z80regs z80;
#include "z80_macros.h"
extern t_CPC CPC;

OpCode::OpCode(int value, int length, int argsize, std::string instruction) :
  value_(value), length_(length), argsize_(argsize), instruction_(std::move(instruction)) {}

uint64_t DisassembledCode::hash() const {
  uint64_t h = 0;
  int i = 0;
  for (const auto& line : lines) {
    h += i * (line.address_ + line.opcode_);
    i += 1;
  }
  return h;
}

bool operator<(const DisassembledLine& l, const DisassembledLine& r) {
  return l.address_ < r.address_;
}

std::map<int, OpCode> load_opcodes_table()
{
  std::map<int, OpCode> opcode_to_instruction;
  std::ifstream infile(CPC.resources_path + "/z80_opcodes.txt");
  std::string line;
  while (std::getline(infile, line)) {
    auto delimiter = line.find(':');
    std::string opcode = line.substr(0, delimiter);
    // -2 for the 0x prefix, divide by 2 to get number of bytes
    int opcode_length = (opcode.length() - 2) / 2;
    int opcode_value = std::stol(opcode, nullptr, 0);
    std::string instruction = line.substr(delimiter+1);
    int nbargs = std::count(instruction.begin(), instruction.end(), '*');
    opcode_to_instruction[opcode_value] = OpCode(opcode_value, opcode_length, nbargs, instruction);
    //std::cout << opcode << "(" << std::hex << opcode_value << "): " << instruction << std::endl;
  }
  return opcode_to_instruction;
}

void add_if_new(word address, const DisassembledCode& result, std::vector<dword>& to_disassemble_from, const std::string& why, word from)
{
  DisassembledLine fakeLine(address, 0, "");
  if (result.lines.count(fakeLine) == 0) {
    to_disassemble_from.push_back(address);
    LOG_VERBOSE("Adding " << std::hex << address << " from " << why << " at " << from);
  }
}

// We use a dword for pos to allow to check if we're reaching the end of the memory
void disassemble_from(dword pos, DisassembledCode& result, std::vector<dword>& to_disassemble_from)
{
  static auto opcode_to_instructions = load_opcodes_table();
  word original_start = pos;
  while (pos <= 0xFFFF) {
    uint64_t opcode = 0;
    word start_address = pos;
    bool found = false;
    for (int bytes_read = 0; bytes_read < 3; bytes_read++) {
      opcode = (opcode << 8) + z80_read_mem(pos++);
      //std::cout << "Looking for opcode " << std::hex << opcode << std::endl;
      if (opcode_to_instructions.find(opcode) != opcode_to_instructions.end()) {
        auto instr = opcode_to_instructions[opcode];
        std::string instruction = instr.instruction_;
        if (instruction.find("**") != std::string::npos) {
         int op = z80_read_mem(pos++);
          op += (z80_read_mem(pos++) << 8);
          opcode = (opcode << 16) + op;
          std::ostringstream oss;
          oss << "$" << std::hex << op;
          instruction.replace(instruction.find("**"), 2, oss.str());
          if (instruction.rfind("call", 0) == 0 ||
              instruction.rfind("jp", 0) == 0) {
            add_if_new(op, result, to_disassemble_from, instruction, start_address);
          }
        }
        while (instruction.find('*') != std::string::npos) {
          auto op = z80_read_mem(pos++);
          opcode = (opcode << 8) + op;
          std::ostringstream oss;
          oss << "$" << std::hex << static_cast<int>(op);
          instruction.replace(instruction.find('*'), 1, oss.str());
          if (instruction.rfind("jr", 0) == 0 ||
              instruction.rfind("djnz", 0) == 0) {
            word address = pos + static_cast<int8_t>(op);
            add_if_new(address, result, to_disassemble_from, instruction, start_address);
          }
          if (instruction.rfind("rst", 0) == 0) {
            // RST instruction is of the form rst xxh where xx can be 00, 08,
            // 10, 18, 20, 28, 30 or 38
            word address = std::stol(instruction.substr(4,2), nullptr, 16);
            add_if_new(address, result, to_disassemble_from, instruction, start_address);
          }
        }
        // TODO: Detect inconsistencies. This requires checking the instructions
        // before and after the newly emplaced one.
        result.lines.emplace(start_address, opcode, std::move(instruction));
        if (instruction == "ret") return;
        found = true;
        break;
      }
    }
    if (!found) {
      // TODO(ColinPitrat): Handle inconsistency
      LOG_VERBOSE("No opcode found at " << std::hex << start_address << " for " << opcode << " from " << original_start);
      uint64_t value = z80_read_mem(start_address);
      std::ostringstream oss;
      oss << "db $" << std::hex << value;
      result.lines.emplace(start_address, value, oss.str());
    }
  }
}

// Ideas:
//  - Disassemble starting at PC
//  - Recursively:
//    - Follow:
//      - jp, call (absolutes)
//      - jr, djnz (relatives)
//      - rst (one byte corresponding to LSB, HSB = 0)
//    - Stop on ret (but not conditional ones), reti, retn (and more?)
//  - Also disassemble starting at a given entry point
//  - Detect inconsistencies
//    - When inserting an instruction, verify it doesn't overlap with the one before or after
//  - On inconsistency with already disassembled code:
//    - From PC, it's probably a bug in the disassembler?
//    - From entry point, it's probably a bad entry point?
DisassembledCode disassemble(const std::vector<word>& entry_points)
{
  DisassembledCode code;
  std::vector<dword> to_disassemble_from(entry_points.begin(), entry_points.end());
  //to_disassemble_from.push_back(_PC);
  while (!to_disassemble_from.empty()) {
    auto next_pos = to_disassemble_from.back();
    to_disassemble_from.pop_back();
    disassemble_from(next_pos, code, to_disassemble_from);
  }
  return code;
}
