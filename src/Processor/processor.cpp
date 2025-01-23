#include <iostream>

#include "processor.hpp"
#include "opcodes.hpp"

namespace gb {

dword Processor::BC() {
  return twoWordToDword(B, C);
}

void Processor::BC(dword value) {
  B = dwordMsb(value);
  C = dwordLsb(value);
}

dword Processor::DE() {
  return twoWordToDword(D, E);
}

void Processor::DE(dword value) {
  D = dwordMsb(value);
  E = dwordLsb(value);
}

dword Processor::HL() {
  return twoWordToDword(H, L);
}

void Processor::HL(dword value) {
  H = dwordMsb(value);
  L = dwordLsb(value);
}

void Processor::setPC(word msb, word lsb) {
  SP = twoWordToDword(msb, lsb);
}

void Processor::setSP(word msb, word lsb) {
  SP = twoWordToDword(msb, lsb);
}

Processor::Processor() = default;

void Processor::connectMemory(Memory* ram) {
  ram_ = ram;
}

void Processor::printRegisters() {
  std::printf("PC: %04X\n", PC);
  std::printf(" A: %02X\n", A);
  std::printf(" F: %02X\n", F);
  std::printf(" B: %02X\n", B);
  std::printf(" C: %02X\n", C);
  std::printf(" D: %02X\n", D);
  std::printf(" E: %02X\n", E);
  std::printf(" H: %02X\n", H);
  std::printf(" L: %02X\n", L);
  std::printf("SP: %04X\n", SP);
  std::printf("\n");
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
  Opcode opcode = (Opcode)ram_->read(PC);
  PC += 1;
  executeOpcode(opcode);

  busyCycles = getBusyCycles(opcode);
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


}
