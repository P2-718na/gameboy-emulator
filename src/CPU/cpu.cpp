#include <iostream>
#include <limits>
#include <cassert>

#include "cpu.hpp"
#include "gameboy.hpp"
#include "opcodes.hpp"
#include "timings.hpp"

namespace gb {

std::array<int, 256> CPU::timings_{};
std::array<int, 256> CPU::timingsCB_{};
std::array<dword, 5> CPU::interruptAddresses;

std::bitset<5> CPU::IE() const {
  return bus->read(0xFFFF) & 0b11111;
}
std::bitset<5> CPU::IF() const {
  return bus->read(0xFF0F) & 0b11111;
}
void CPU::IF(FlagInterrupt interrupt, bool enabled) {
  auto flags = IF();
  flags[interrupt] = enabled;
  bus->write(0xFF0F, flags.to_ulong());
}

dword CPU::BC() const {
  return twoWordToDword(B, C);
}

void CPU::BC(word msb, word lsb) {
  B = msb;
  C = lsb;
}
void CPU::BC(dword value) {
  BC(
    dwordMsb(value),
    dwordLsb(value)
  );
}

dword CPU::DE() const {
  return twoWordToDword(D, E);
}

void CPU::DE(word msb, word lsb) {
  D = msb;
  E = lsb;
}
void CPU::DE(dword value) {
  DE(
    dwordMsb(value),
    dwordLsb(value)
  );
}

dword CPU::HL() const {
  return twoWordToDword(H, L);
}

void CPU::HL(word msb, word lsb) {
  H = msb;
  L = lsb;
}

void CPU::HL(dword value) {
  HL(dwordMsb(value), dwordLsb(value));
}

word CPU::iHL() const {
  return bus->read(HL());
}

void CPU::iHL(word value) {
  bus->write(HL(), value);
}

void CPU::incRegister(word& reg) {
  F[FH] = getHalfCarryFlag(reg, 1);
  reg += 1;
  F[FZ] = reg == 0;
  F[FN] = false;
}
void CPU::decRegister(word& reg) {
  // todo check negative carry
  F[FH] = getHalfCarryFlag(reg, -1);
  reg -= 1;
  F[FZ] = reg == 0;
  F[FN] = true;
}

void CPU::addRegister(word reg) {
  F[FC] = getCarryFlag(A, reg);
  F[FH] = getHalfCarryFlag(A, reg);
  A += reg;
  F[FZ] = A == 0;
  F[FN] = false;
}
void CPU::subRegister(word reg) {
  //Todo sub
  F[FC] = reg < A;
  F[FH] = getHalfCarryFlag(A, -reg);
  A -= reg;
  F[FZ] = A == 0;
  F[FN] = true;
}

void CPU::andRegister(word reg) {
  A &= reg;
  F[FZ] = A == 0;
  F[FC] = false;
  F[FH] = true;
  F[FN] = false;
}
void CPU::orRegister(word reg) {
  A |= reg;
  F[FZ] = A == 0;
  F[FC] = false;
  F[FH] = false;
  F[FN] = false;
}


void CPU::adcRegister(word reg) {
  // TODO poorly documented carry bits, check
  const word result = A + reg + F[FC];
  F[FC] = getCarryFlag(A, reg + F[FC]) || getCarryFlag(reg, F[FC]);
  F[FH] = getHalfCarryFlag(A, reg + F[FC]) || getHalfCarryFlag(reg, F[FC]);
  A = result;
  F[FZ] = A == 0;
  F[FN] = false;
}
void CPU::sbcRegister(word reg) {
  // TODO poorly documented carry bits, check
  const word result = A - reg - F[FC];
  // static casts here prevent ambiguity
  F[FC] = (reg + F[FC]) > A;
  F[FH] = getHalfCarryFlag(A, -reg - F[FC]) || getHalfCarryFlag(static_cast<word>(-reg), -F[FC]);
  A = result;
  F[FZ] = A == 0;
  F[FN] = true;
}

void CPU::xorRegister(gb::word reg) {
    A ^= reg;
    F[FZ] = reg == 0;
    F[FN] = false;
    F[FH] = false;
    F[FC] = false;
}

// The flag for cp, sub, sbc behaves differently than what is specified in official docs.
// https://stackoverflow.com/questions/31409444/what-is-the-behavior-of-the-carry-flag-for-cp-on-a-game-boy
// https://forums.nesdev.org/viewtopic.php?t=12861
void CPU::cpRegister(word reg) {
  // Fixme fix carry bits
  const word result = A - reg;
  F[FH] = getHalfCarryFlag(A, -reg);
  F[FC] = reg > A;
  F[FZ] = result == 0;
  F[FN] = true;
}



void CPU::setSP(word msb, word lsb) {
  SP = twoWordToDword(msb, lsb);
}

void CPU::setPC(word msb, word lsb) {
  PC = twoWordToDword(msb, lsb);
}

void CPU::ret(bool condition) {
  if (!condition) {
    return;
  }

  const auto lsb = popSP();
  const auto msb = popSP();
  setPC(msb, lsb);
}

void CPU::jr(bool condition) {
  const auto e = popPCSigned();
  if (condition) {
    PC += e;
    ++busyCycles;
  }
};

void CPU::jpImm(bool condition) {
  auto lsb = popPC();
  auto msb = popPC();

  if (condition) {
    setPC(msb, lsb);
  }
}

void CPU::callImm(bool condition) {
  auto lsb = popPC();
  auto msb = popPC();

  if (condition) {
    pushPCToStack();
    setPC(msb, lsb);
  }
}

void CPU::loadImm(word& reg) {
  reg = popPC();
}

word CPU::popPC() {
  return bus->read(PC++);
}

word CPU::popSP() {
  return bus->read(SP++);
}

signed char CPU::popPCSigned() {
  return static_cast<signed char>(bus->read(PC++));
}

void CPU::pushPCToStack() {
  bus->write(--SP, dwordMsb(PC));
  bus->write(--SP, dwordLsb(PC));
}


// todo all these classes should be derived class and call super constructor to set ram and gameboy.
CPU::CPU(Gameboy* gameboy, AddressBus* ram) : GBComponent{gameboy, ram} {
  initTimings();
  initTimingsCB();

  interruptAddresses[VBlankBit] = 0x40;
  interruptAddresses[STATBit]   = 0x48;
  interruptAddresses[TimerBit]  = 0x50;
  interruptAddresses[SerialBit] = 0X58;
  interruptAddresses[JoypadBit] = 0x60;
}

void CPU::reset() {
  A = 0x01;
  F = 0x00;
  B = 0xFF;
  C = 0x13;
  D = 0x00;
  E = 0xC1;
  H = 0x84;
  L = 0x03;
  PC = 0x0100;
  SP = 0xFFFE;
}

void CPU::printRegisters() {
  std::printf("__CPU_________________________________________________________________\n");
  std::printf("|  PC  | OC | A  | F  | B  | C  | D  | E  | H  | L  |  SP  | H | IME |\n");
  std::printf("| %04X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %02X | %04X | %C |  %C  |\n",
    PC,
    bus->read(PC),
    A,
    static_cast<int>(F.to_ulong()),
    B,
    C,
    D,
    E,
    H,
    L,
    SP,
    halted_ ? 'T' : 'F',
    IME ? 'T' : 'F'
  );
  std::printf("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n");
}

void CPU::printRegistersIfChanged() {
  if (busyCycles == 0) {
    printRegisters();
  }
}

void CPU::machineClock() {
  assert(busyCycles >= 0);

  if (crashed) {
    return;
  }

  if (busyCycles != 0) {
    --busyCycles;
    return;
  }

  // Interrupts are only fetched at the end of current instruction
  // Todo handle HALT bug.
  const auto wasInterruptHandled = handleInterrupts();

  if (wasInterruptHandled) {
    return;
  }
  if (halted_) {
    return;
  }

  executeCurrentInstruction();
}

void CPU::executeCurrentInstruction() {
  assert(busyCycles == 0);

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

bool CPU::handleInterrupts() {
  assert(busyCycles == 0);

  // Master interrupt switch
  if (!IME && !halted_) {
    return false;
  }

  const auto interruptEnabled = IE();
  // No specific interrupts are active
  if (interruptEnabled == 0) {
    return false;
  }
  const auto interruptRequested = IF();
  // No interrupts are requested
  if (interruptRequested == 0) {
    return false;
  }

  for (int i = 0; i != 5; ++i) {
    bool requestedInterrupt = interruptRequested[i];
    bool enabledInterrupt   = interruptEnabled[i];

    if (requestedInterrupt && enabledInterrupt) {
      triggerInterrupt(static_cast<FlagInterrupt>(i));
      halted_ = false;
      // Only the interrupt with the highest priority gets handled.
      return true;
    }
  }

  return false;
}

void CPU::triggerInterrupt(const FlagInterrupt interrupt) {
  assert(busyCycles == 0 && "Interrupts cannot be handled while an instruction is still running.");

  // Need to check here due to halt stuff
  if (!IME) {
    return;
  }

  // Pause interrupt and reset the flag for this specific interrupt.
  IME = false;
  IF(interrupt, false);

  // Interrupt routine takes 5 machine cycles to complete. Then, code execution continues as normal.
  pushPCToStack();
  PC = interruptAddresses[interrupt];
  busyCycles = 5;
}

dword CPU::twoWordToDword(word msb, word lsb) {
  // todo add tests and check type conversion stuff prob implicit cast not deeded here
  dword result = msb;
  result <<= 8;
  result += lsb;
  return result;
}

word CPU::dwordLsb(dword value) {
  return value & 0b11111111;
}

word CPU::dwordMsb(dword value) {
  return (value >> 8) & 0b11111111;
}

bool CPU::nthBit(word byte, int bit) {
  assert(bit <= 7);

  return (byte & (1 << bit)) != 0;
}

bool CPU::breakpoint() const {
  return breakpoint_;
}

// fixme these two are ugly vv
bool CPU::getCarryFlag(word a, word b) {
  static const int maxValue = std::numeric_limits<word>::max();
  const int result = static_cast<int>(a) + static_cast<int>(b);
  return result > maxValue;
}
bool CPU::getCarryFlag(dword a, dword b) {
  static const int maxValue = std::numeric_limits<dword>::max();
  const int result = static_cast<int>(a) + static_cast<int>(b);
  return result > maxValue;
}

// Thx mommy
// https://gist.github.com/meganesu/9e228b6b587decc783aa9be34ae27841
bool CPU::getHalfCarryFlag(word a, word b) {
  return (((a & 0xF) + (b & 0xF)) & 0x10) == 0x10;
}
bool CPU::getHalfCarryFlag(dword a, dword b) {
  return (((a & 0xFFF) + (b & 0xFFF)) & 0x1000) == 0x1000;
}

}
