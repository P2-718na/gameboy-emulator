#include <iostream>

#include "processor.hpp"
#include "timings.hpp"
#include "opcodes.hpp"

namespace gb {

dword Processor::BC() {
  return twoWordToDword(B, C);
}

void Processor::BC(word msb, word lsb) {
  B = msb;
  C = lsb;
}
void Processor::BC(dword value) {
  BC(
    dwordMsb(value),
    dwordLsb(value)
  );
}

dword Processor::DE() {
  return twoWordToDword(D, E);
}

void Processor::DE(word msb, word lsb) {
  D = msb;
  E = lsb;
}
void Processor::DE(dword value) {
  DE(
    dwordMsb(value),
    dwordLsb(value)
  );
}

dword Processor::HL() {
  return twoWordToDword(H, L);
}

void Processor::HL(word msb, word lsb) {
  H = msb;
  L = lsb;
}
void Processor::HL(dword value) {
  HL(
    dwordMsb(value),
    dwordLsb(value)
  );
}

void Processor::setPC(word msb, word lsb) {
  SP = twoWordToDword(msb, lsb);
}

void Processor::setSP(word msb, word lsb) {
  SP = twoWordToDword(msb, lsb);
}

word Processor::popPC() {
  return ram_->read(PC++);
}


Processor::Processor() = default;

void Processor::connectMemory(Memory* ram) {
  ram_ = ram;
}

void Processor::printRegisters() {
  std::printf("____________________________________________________________\n");
  std::printf("|  PC  | OC | A  | F  | B  | C  | D  | E  | H  | L  |  SP  |\n");
  std::printf("| %04X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %04X |\n",
    PC,
    ram_->read(PC),
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    SP
  );
  std::printf("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n");
}

void Processor::printRegistersIfChanged() {
  if (busyCycles == 0) {
    printRegisters();
  }
}


void Processor::machineClock() {
  if (busyCycles > 0) {
    --busyCycles;
    return;
  }

  // todo proper casting
  const auto opcode = (Opcode)ram_->read(PC++);

  if (opcode != CB) {
    // busyCycles needs to be set before executing opcode as conditional jumps may
    // increase its value
    busyCycles = getBusyCycles(opcode);
    executeOpcode(opcode);
    return;
  }

  // busyCycles needs to be set before executing opcode as conditional jumps may
  // increase its value
  // todo proper casting
  const auto cbOpcode = (CBOpcode)ram_->read(PC++);
  busyCycles = getBusyCyclesCB(cbOpcode);
  executeCBOpcode(cbOpcode);
};


dword Processor::twoWordToDword(word msb, word lsb) {
  // todo add tests and check type conversion stuff prob implicit cast not deeded here
  dword result = msb;
  result <<= 8;
  result += lsb;
  return result;
}

word Processor::dwordLsb(dword value) {
  return value & 0b11111111;
}

word Processor::dwordMsb(dword value) {
  return (value >> 8) & 0b11111111;
}

bool Processor::nthBit(word byte, int bit) {
  assert(bit <= 7);

  return (byte & (1 << bit)) != 0;
}


}
