#include "z80_disassembly.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
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

std::optional<DisassembledLine> DisassembledCode::LineAt(word address) const {
  for (const auto& l : lines) {
    if (l.address_ == address) {
      return l;
    }
  }
  return {};
}

std::ostream& operator<<(std::ostream& os, const DisassembledLine& line)
{
  os << std::setfill('0') << std::setw(4) << std::hex << line.address_ << ": ";
  os << std::setfill(' ') << std::setw(8) << line.opcode_ << " " << line.instruction_;
  return os;
}

std::ostream& operator<<(std::ostream& os, const DisassembledCode& code)
{
  for (const auto& line : code.lines)
  {
    os << line << std::endl;
  }
  return os;
}

DisassembledLine::DisassembledLine(word address, uint64_t opcode, std::string&& instruction, int64_t ref_address) :
      address_(address), opcode_(opcode), instruction_(instruction)
{
  if (ref_address >= 0) {
    ref_address_ = ref_address;
    std::ostringstream oss;
    oss << "$" << std::hex << std::setw(4) << std::setfill('0') << ref_address;
    ref_address_string_ = oss.str();
  }
}

int DisassembledLine::Size() const
{
  if (opcode_ < 0x100) return 1;
  if (opcode_ < 0x10000) return 2;
  if (opcode_ < 0x1000000) return 3;
  if (opcode_ < 0x100000000) return 4;
  if (opcode_ < 0x10000000000) return 5;
  if (opcode_ < 0x1000000000000) return 6;
  if (opcode_ < 0x100000000000000) return 7;
  return 8;
}

bool operator<(const DisassembledLine& l, const DisassembledLine& r) {
  return l.address_ < r.address_;
}

bool operator==(const DisassembledLine& l, const DisassembledLine& r) {
  return l.address_ == r.address_ && l.opcode_ == r.opcode_ && l.instruction_ == r.instruction_;
}

std::map<int, OpCode> load_opcodes_table()
{
  std::map<int, OpCode> opcode_to_instruction;
  std::ifstream infile(CPC.resources_path + "/z80_opcodes.txt");
  if (!infile.good()) {
    LOG_ERROR("Error opening " << CPC.resources_path << "/z80_opcodes.txt");
  }
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

void append_address(std::string& instruction, word address)
{
  std::ostringstream oss;
  oss << "  ; $" << std::hex << std::setw(4) << std::setfill('0') << address;
  instruction += oss.str();
}

DisassembledLine disassemble_one(dword start_address, DisassembledCode& result, std::vector<dword>& called_points)
{
  static auto opcode_to_instructions = load_opcodes_table();
  uint64_t opcode = 0;
  word pos = start_address;
  for (int bytes_read = 0; bytes_read < 3; bytes_read++) {
    int64_t ref_address = -1;
    opcode = (opcode << 8) + z80_read_mem(pos++);
    //std::cout << "Looking for opcode " << std::hex << opcode << std::endl;
    if (opcode_to_instructions.find(opcode) != opcode_to_instructions.end()) {
      auto instr = opcode_to_instructions[opcode];
      std::string instruction = instr.instruction_;
      if (instruction.find("**") != std::string::npos) {
        int op = z80_read_mem(pos++);
        opcode = (opcode << 8) + op;
        int second_byte = z80_read_mem(pos++);
        op += (second_byte << 8);
        opcode = (opcode << 8) + second_byte;
        std::ostringstream oss;
        oss << "$" << std::hex << std::setw(4) << std::setfill('0') << op;
        instruction.replace(instruction.find("**"), 2, oss.str());
        if (instruction.rfind("call", 0) == 0 ||
            instruction.rfind("jp", 0) == 0) {
          add_if_new(op, result, called_points, instruction, start_address);
          ref_address = op;
        }
      }
      while (instruction.find('*') != std::string::npos) {
        auto op = z80_read_mem(pos++);
        opcode = (opcode << 8) + op;
        std::ostringstream oss;
        oss << "$" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(op);
        instruction.replace(instruction.find('*'), 1, oss.str());
        if (instruction.rfind("jr", 0) == 0 ||
            instruction.rfind("djnz", 0) == 0) {
          word address = pos + static_cast<int8_t>(op);
          append_address(instruction, address);
          add_if_new(address, result, called_points, instruction, start_address);
          ref_address = address;
        }
        if (instruction.rfind("rst", 0) == 0) {
          // RST instruction is of the form rst xxh where xx can be 00, 08,
          // 10, 18, 20, 28, 30 or 38
          word address = std::stol(instruction.substr(4,2), nullptr, 16);
          append_address(instruction, address);
          add_if_new(address, result, called_points, instruction, start_address);
          ref_address = address;
        }
      }
      // TODO: Detect inconsistencies (overlapping instructions). This
      // requires checking the instructions before and after the newly
      // emplaced one.
      return DisassembledLine(start_address, opcode, std::move(instruction), ref_address);
    }
  }
  LOG_VERBOSE("No opcode found at " << std::hex << start_address << " for " << opcode << " from " << start_address);
  uint64_t value = z80_read_mem(start_address);
  std::ostringstream oss;
  oss << "db $" << std::hex << value;
  return DisassembledLine(start_address, value, oss.str());
}

// We use a dword for pos to allow to check if we're reaching the end of the memory
void disassemble_from(dword pos, DisassembledCode& result, std::vector<dword>& to_disassemble_from)
{
  while (pos <= 0xFFFF) {
    auto line = disassemble_one(pos, result, to_disassemble_from);
    pos += line.Size();
    result.lines.insert(line);
    if (line.instruction_ == "ret") return;
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
