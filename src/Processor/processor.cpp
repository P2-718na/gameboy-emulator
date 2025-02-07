#include <iostream>
#include <limits>

#include "processor.hpp"
#include "timings.hpp"
#include "opcodes.hpp"

namespace gb {

std::array<int, 256> Processor::timings_{};
std::array<int, 256> Processor::timingsCB_{};

void Processor::IME(bool status) {
  // TODO SET INTERRUPT
  printf("INTERRUPT LOGIC MISSING");
}

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

dword Processor::HL() const {
  return twoWordToDword(H, L);
}

void Processor::HL(word msb, word lsb) {
  H = msb;
  L = lsb;
}

void Processor::HL(dword value) {
  HL(dwordMsb(value), dwordLsb(value));
}

word Processor::iHL() const {
  return ram_->read(HL());
}

void Processor::iHL(word value) {
  ram_->write(HL(), value);
}

void Processor::incRegister(word& reg) {
  F[FH] = getHalfCarryFlag(reg, 1);
  reg += 1;
  F[FZ] = reg == 0;
  F[FN] = false;
}
void Processor::decRegister(word& reg) {
  // todo check negative carry
  F[FH] = getHalfCarryFlag(reg, -1);
  reg -= 1;
  F[FZ] = reg == 0;
  F[FN] = true;
}

void Processor::addRegister(word reg) {
  F[FC] = getCarryFlag(A, reg);
  F[FH] = getHalfCarryFlag(A, reg);
  A += reg;
  F[FZ] = A == 0;
  F[FN] = false;
}
void Processor::subRegister(word reg) {
  //Todo sub
  F[FC] = getCarryFlag(A, -reg);
  F[FH] = getHalfCarryFlag(A, -reg);
  A -= reg;
  F[FZ] = A == 0;
  F[FN] = true;
}

void Processor::andRegister(word reg) {
  A &= reg;
  F[FZ] = A == 0;
  F[FC] = false;
  F[FH] = true;
  F[FN] = false;
}
void Processor::orRegister(word reg) {
  A |= reg;
  F[FZ] = A == 0;
  F[FC] = false;
  F[FH] = false;
  F[FN] = false;
}


void Processor::adcRegister(word reg) {
  // TODO poorly documented carry bits, check
  const word result = A + reg + F[FC];
  F[FC] = getCarryFlag(A, reg + F[FC]) || getCarryFlag(reg, F[FC]);
  F[FH] = getHalfCarryFlag(A, reg + F[FC]) || getHalfCarryFlag(reg, F[FC]);
  A = result;
  F[FZ] = A == 0;
  F[FN] = false;
}
void Processor::sbcRegister(word reg) {
  // TODO poorly documented carry bits, check
  const word result = A - reg - F[FC];
  // static casts here prevent ambiguity
  F[FC] = getCarryFlag(A, -reg - F[FC]) || getCarryFlag(static_cast<word>(-reg), -F[FC]);
  F[FH] = getHalfCarryFlag(A, -reg - F[FC]) || getHalfCarryFlag(static_cast<word>(-reg), -F[FC]);
  A = result;
  F[FZ] = A == 0;
  F[FN] = true;
}

void Processor::xorRegister(gb::word reg) {
    A ^= reg;
    F[FZ] = reg == 0;
    F[FN] = false;
    F[FH] = false;
    F[FC] = false;
}

void Processor::cpRegister(word reg) {
  // Fixme fix carry bits
  F[FH] = getHalfCarryFlag(A, -reg);
  F[FC] = getCarryFlag(A, -reg);
  A -= reg;
  F[FZ] = A == 0;
  F[FN] = true;
}



void Processor::setSP(word msb, word lsb) {
  SP = twoWordToDword(msb, lsb);
}

void Processor::setPC(word msb, word lsb) {
  PC = twoWordToDword(msb, lsb);
}

void Processor::ret(bool condition) {
  if (!condition) {
    return;
  }

  const auto lsb = popSP();
  const auto msb = popSP();
  setPC(msb, lsb);
}

void Processor::jr(bool condition) {
  const auto e = popPCSigned();
  if (condition) {
    PC += e;
    ++busyCycles;
  }
};

void Processor::jpImm(bool condition) {
  auto lsb = popPC();
  auto msb = popPC();

  if (condition) {
    setPC(msb, lsb);
  }
}

void Processor::callImm(bool condition) {
  auto lsb = popPC();
  auto msb = popPC();

  if (condition) {
    pushPCToStack();
    setPC(msb, lsb);
  }
}

void Processor::loadImm(word& reg) {
  reg = popPC();
}

word Processor::popPC() {
  return ram_->read(PC++);
}

word Processor::popSP() {
  return ram_->read(SP++);
}

signed char Processor::popPCSigned() {
  return static_cast<signed char>(ram_->read(PC++));
}

void Processor::pushPCToStack() {
  ram_->write(--SP, dwordMsb(PC));
  ram_->write(--SP, dwordLsb(PC));
}


// todo handle ownership of things better
Processor::Processor(Memory* ram) : ram_{ram} {
  initTimings();
  initTimingsCB();
}

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

void Processor::crash() {
  // Todo handle proper crash logic
  printf("Processor enccountered UNDEFINED opcode. Terminating.\n");
  exit(1);
}


void Processor::executeCurrentInstruction() {
  assert(busyCycles >= 0);

  if (busyCycles != 0) {
    --busyCycles;
    return;
  }

  // todo proper casting
  const auto opcode = static_cast<Opcode>(popPC());

  if (opcode != CB) {
    // busyCycles needs to be set before executing opcode as conditional jumps may increase its value
    busyCycles = getBusyCycles(opcode);
    executeOpcode(opcode);
    return;
  }

  // busyCycles needs to be set before executing opcode as conditional jumps may
  // increase its value
  const auto cbOpcode = static_cast<CBOpcode>(popPC());
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

// fixme these two are ugly vv
bool Processor::getCarryFlag(word a, word b) {
  static const int maxValue = std::numeric_limits<word>::max();
  const int result = static_cast<int>(a) + static_cast<int>(b);
  return result > maxValue;
}
bool Processor::getCarryFlag(dword a, dword b) {
  static const int maxValue = std::numeric_limits<dword>::max();
  const int result = static_cast<int>(a) + static_cast<int>(b);
  return result > maxValue;
}

// Thx mommy
// https://gist.github.com/meganesu/9e228b6b587decc783aa9be34ae27841
bool Processor::getHalfCarryFlag(word a, word b) {
  return (((a & 0xF) + (b & 0xF)) & 0x10) == 0x10;
}
bool Processor::getHalfCarryFlag(dword a, dword b) {
  return (((a & 0xFFF) + (b & 0xFFF)) & 0x1000) == 0x1000;
}

}
