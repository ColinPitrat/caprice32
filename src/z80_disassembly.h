#ifndef Z80_DISASSEMBLY_H
#define Z80_DISASSEMBLY_H

#include "types.h"
#include <map>
#include <optional>
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

    int Size() const;

    friend bool operator<(const DisassembledLine& l, const DisassembledLine& r);
    friend bool operator==(const DisassembledLine& l, const DisassembledLine& r);

    word address_;
    uint64_t opcode_;
    std::string instruction_;
    word ref_address_ = 0;
    std::string ref_address_string_;
};

std::ostream& operator<<(std::ostream& os, const DisassembledLine& line);

class DisassembledCode {
  public:
    DisassembledCode() = default;

    std::optional<DisassembledLine> LineAt(word address) const;

    uint64_t hash() const;

    std::set<DisassembledLine> lines;
};

std::ostream& operator<<(std::ostream& os, const DisassembledCode& code);

std::map<int, OpCode> load_opcodes_table();
DisassembledLine disassemble_one(dword pos, DisassembledCode& result, std::vector<dword>& entry_points);
DisassembledCode disassemble(const std::vector<word>& entry_points);

#endif
