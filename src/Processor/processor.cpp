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
  HL(dwordMsb(value), dwordLsb(value));
}

void Processor::incrementRegister(word& reg) {
  // TODO this definition is sus but it appears correct.
  bool bit3 = nthBit(reg, 3);
  reg += 1;
  bool carry3 = (bit3 == 1 && nthBit(reg, 3) == 0);  // Todo add tests
  F[FZ] = reg == 0;
  F[FN] = false;
  F[FH] = carry3;  // see this is set if there is a carry per bit 3 (??)
}

void Processor::decrementRegister(word& reg) {
  // TODO this definition is sus but it appears correct.
  bool bit3 = nthBit(reg, 3);
  reg -= 1;
  bool carry3 = (bit3 == 1 && nthBit(reg, 3) == 0); // Todo I am not really sure what carry means in the context of subtraction
  F[FZ] = reg == 0;
  F[FN] = true;
  F[FH] = carry3;
}

void Processor::subRegister(word& reg) {
  // TODO this definition is sus but it appears correct.
  bool bit3 = nthBit(A, 3);
  bool bit7 = nthBit(A, 7);
  // FIXME write result,carry_per_bit function
  A -= reg;
  bool carry3 = (bit3 == 1 && nthBit(A, 3) == 0); // Todo I am not really sure what carry means in the context of subtraction
  F[FZ] = A == 0;
  F[FN] = true;
  F[FH] = carry3;
  F[FC] = bit7 == 1 && nthBit(A, 7) == 0; // Todo test this
}

void Processor::cmpRegister(word reg) {
  // Fixme fix carry bits
  const word result = A - reg;
  F[FZ] = result == 0;
  F[FN] = true;
  F[FH] = nthBit(A, 3) == 0 && nthBit(reg, 3) == 1; // Todo test this
  F[FC] = nthBit(A, 7) == 0 && nthBit(reg, 7) == 1; // Todo test this
  // For this it says that it checks the borrow bit and not carry
  /// 10000 -
  /// 01000 =
  /// -------
  ///  1000
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


// todo handle ownership of things better
Processor::Processor(Memory* ram) : ram_{ram} {}

void Processor::printRegisters() {
  std::printf("__CPU_______________________________________________________\n");
  std::printf("|  PC  | OC | A  | F  | B  | C  | D  | E  | H  | L  |  SP  |\n");
  std::printf("| %04X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %04X |\n",
    PC,
    ram_->read(PC),
    A,
    static_cast<int>(F.to_ulong()),
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
  // Todo
  executeCurrentInstruction();
  // updateTimers();
  // updateGraphics();
  handleInterrupts();
}


void Processor::executeCurrentInstruction() {
  if (busyCycles > 0) {
    --busyCycles;
    return;
  }

  // todo proper casting
  const auto opcode = (Opcode)popPC();

  if (opcode != CB) {
    // busyCycles needs to be set before executing opcode as conditional jumps may increase its value
    busyCycles = getBusyCycles(opcode);
    executeOpcode(opcode);
    return;
  }

  // busyCycles needs to be set before executing opcode as conditional jumps may
  // increase its value
  // todo proper casting
  const auto cbOpcode = (CBOpcode)popPC();
  busyCycles = getBusyCyclesCB(cbOpcode);
  executeCBOpcode(cbOpcode);
};

void Processor::handleInterrupts() {
  if (!interrupt_) {
    return;
  }

  // Todo interrupt logic
}

void Processor::setInterrupt(gb::dword address) {
  interrupt_ = true;

  // todo interrupt set logic
}

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

bool Processor::breakpoint() const {
  return breakpoint_;
}

}
