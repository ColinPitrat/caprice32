#ifndef Z80_DISASSEMBLY_H
#define Z80_DISASSEMBLY_H

#include "types.h"
#include <map>
#include <set>
#include <string>
#include <vector>

class OpCode {
  public:
    OpCode() = default;
    OpCode(int value, int length, int argsize, std::string instruction);

    int value_;
    int length_;
    int argsize_;
    std::string instruction_;
};

class DisassembledLine {
  public:
    DisassembledLine(word address, uint64_t opcode, std::string&& instruction, int64_t ref_address = -1);

    void AddRefAddress(word address);

    friend bool operator<(const DisassembledLine& l, const DisassembledLine& r);

    word address_;
    uint64_t opcode_;
    std::string instruction_;
    word ref_address_ = 0;
    std::string ref_address_string_;
};

class DisassembledCode {
  public:
    DisassembledCode() = default;

    uint64_t hash() const;

    std::set<DisassembledLine> lines;
};

std::map<int, OpCode> load_opcodes_table();
DisassembledCode disassemble(const std::vector<word>& entry_points);

#endif
