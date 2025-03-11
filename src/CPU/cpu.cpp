#include <iostream>
#include <limits>
#include <cassert>

#include "cpu.hpp"
#include "gameboy.hpp"
#include "opcodes.hpp"
#include "timings.hpp"

namespace gb {

std::array<int, 256> CPU::INSTRUCTION_TIMINGS{};
std::array<int, 256> CPU::CB_INSTRUCTION_TIMINGS{};
std::array<dword, 5> CPU::INTERRUPT_JUMP_ADDRESSES;

std::bitset<5> CPU::IE() const {
  return bus->read(REG_IE) & 0b11111;
}
std::bitset<5> CPU::IF() const {
  return bus->read(REG_IF) & 0b11111;
}
void CPU::IF(INTERRUPT_ID interrupt, bool enabled) {
  auto flags = IF();
  flags[interrupt] = enabled;
  bus->write(REG_IF, flags.to_ulong());
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
  F[FH] = (reg & 0xF) - 1 < 0;
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
  F[FH] = (A & 0xF) - (reg & 0xF) < 0;
  F[FC] = A < reg;
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
  const int result = A + reg + F[FC];
  F[FH] = (A & 0xF) + (reg & 0xF) + F[FC] > 0xF;
  F[FC] = result > 0xFF;
  A = result;
  F[FZ] = A == 0;
  F[FN] = false;
}
void CPU::sbcRegister(word reg) {
  const int result = A - reg - F[FC];

  F[FH] = (A & 0xF) - (reg & 0xF) - F[FC] < 0;
  F[FC] = result < 0;
  A = result;
  F[FZ] = A == 0;
  F[FN] = true;
}

void CPU::xorRegister(gb::word reg) {
    A ^= reg;
    F[FZ] = A == 0;
    F[FN] = false;
    F[FH] = false;
    F[FC] = false;
}

// The flag for cp, sub, sbc behaves differently than what is specified in official docs.
// https://stackoverflow.com/questions/31409444/what-is-the-behavior-of-the-carry-flag-for-cp-on-a-game-boy
// https://forums.nesdev.org/viewtopic.php?t=12861
void CPU::cpRegister(word reg) {
  const word result = A - reg;
  F[FH] = (A & 0xf) - (reg & 0xf) < 0;
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
    busyCycles -= 3;
    return;
  }

  const auto lsb = popSP();
  const auto msb = popSP();
  setPC(msb, lsb);
}

void CPU::jr(bool condition) {
  const auto e = popPCSigned();
  if (!condition) {
    --busyCycles;
    return;
  }

  PC += e;
};

void CPU::jpImm(bool condition) {
  auto lsb = popPC();
  auto msb = popPC();

  if (!condition) {
    --busyCycles;
    return;
  }

  setPC(msb, lsb);
}

void CPU::callImm(bool condition) {
  auto lsb = popPC();
  auto msb = popPC();

  if (!condition) {
    busyCycles -= 3;
    return;
  }

  pushPCToStack();
  setPC(msb, lsb);
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

CPU::CPU(Gameboy* gameboy, AddressBus* bus)
  : bus{ bus }
  , gameboy{ gameboy } {
  initTimings();
  initTimingsCB();

  INTERRUPT_JUMP_ADDRESSES[INTERRUPT_VBLANK] = 0x40;
  INTERRUPT_JUMP_ADDRESSES[INTERRUPT_STAT]   = 0x48;
  INTERRUPT_JUMP_ADDRESSES[INTERRUPT_TIMER]  = 0x50;
  INTERRUPT_JUMP_ADDRESSES[INTERRUPT_SERIAL] = 0X58;
  INTERRUPT_JUMP_ADDRESSES[INTERRUPT_JOYPAD] = 0x60;
}

void CPU::executeCurrentInstruction() {
  assert(busyCycles == 0);

  const auto opcode = static_cast<OPCODE>(popPC());

  if (opcode != CB) {
    executeOpcode(opcode);
    return;
  }

  const auto cbOpcode = static_cast<CB_OPCODE>(popPC());
  executeCBOpcode(cbOpcode);
};

bool CPU::tryTriggerInterrupt() {
  assert(busyCycles == 0);

  // Master interrupt switch
  if (!IME && !halted) {
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
      triggerInterrupt(static_cast<INTERRUPT_ID>(i));
      halted = false;
      // Only the interrupt with the highest priority gets handled.
      return true;
    }
  }

  return false;
}

void CPU::triggerInterrupt(const INTERRUPT_ID interrupt) {
  assert(busyCycles == 0 && "Interrupts cannot be handled while an instruction is still running.");

  // We MUST do this check here because
  // interrupts have to disable the halted status, even if IME is
  // false.
  if (!IME) {
    return;
  }

  // Pause interrupts and reset the flag for this specific interrupt.
  IME = false;
  IF(interrupt, false);

  // Interrupt routine takes 5 machine cycles to complete. Then, code execution continues as normal.
  pushPCToStack();
  PC = INTERRUPT_JUMP_ADDRESSES[interrupt];
  busyCycles = 5;
}


void CPU::reset() {
  A  = 0x01;
  F  = 0x00;
  B  = 0xFF;
  C  = 0x13;
  D  = 0x00;
  E  = 0xC1;
  H  = 0x84;
  L  = 0x03;
  PC = 0x0100;
  SP = 0xFFFE;
}

void CPU::printRegisters() const {
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
    // L indicates char is format wchar_t instead of char (aka, unicode char).
    halted ? L'T' : L'F',
    IME ? L'T' : L'F'
  );
  std::printf("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n");
}

void CPU::printRegistersIfChanged() const {
  if (busyCycles == 0) {
    printRegisters();
  }
}

void CPU::machineClock() {
  // busyCycles could be declared as unsigned, but keeping it this way we can
  // check if something strange happened that caused it to go
  // in the negatives.
  assert(busyCycles >= 0);

  if (crashed) {
    return;
  }

  if (busyCycles != 0) {
    --busyCycles;
    return;
  }

  // Interrupts are only fetched at the end of current instruction
  // (that is, when busyCycles = 0).
  const auto wasInterruptTriggered = tryTriggerInterrupt();
  // If interrupt is triggered,
  // this cycle is effectively waster. PC gets moved to the correct location,
  // interrupts are disabled and execution resumes like normal starting from
  // next machine cycle.
  if (wasInterruptTriggered) {
    return;
  }

  // It is important that the halted check happens before tryTriggerInterrupt()
  // as interrupts disable halted status.
  if (halted) {
    return;
  }

  executeCurrentInstruction();
}

dword CPU::twoWordToDword(const word msb, const word lsb) {
  dword result = msb;
  result <<= 8;
  result += lsb;
  return result;
}

word CPU::dwordLsb(const dword value) {
  return value & 0b11111111;
}

word CPU::dwordMsb(const dword value) {
  return (value >> 8) & 0b11111111;
}


int CPU::getBusyCyclesCB(const CB_OPCODE opcode) {
  const int busyCycles = CB_INSTRUCTION_TIMINGS[opcode];
  assert(busyCycles != 0);
  return busyCycles;
}
int CPU::getBusyCycles(const OPCODE opcode) {
  const int busyCycles = INSTRUCTION_TIMINGS[opcode];
  assert(busyCycles != 0);
  return busyCycles;
}

bool CPU::nthBit(const word byte, const int bit) {
  assert(bit <= 7);

  return (byte & (1 << bit)) != 0;
}

bool CPU::getCarryFlag(word a, word b) {
  static constexpr int maxValue = std::numeric_limits<word>::max();
  const int result = static_cast<int>(a) + static_cast<int>(b);
  return result > maxValue;
}
bool CPU::getCarryFlag(dword a, dword b) {
  static constexpr int maxValue = std::numeric_limits<dword>::max();
  const int result = static_cast<int>(a) + static_cast<int>(b);
  return result > maxValue;
}

// Online you'll find an alternative version which does NOT work:
// https://gist.github.com/meganesu/9e228b6b587decc783aa9be34ae27841
bool CPU::getHalfCarryFlag(const word a, const word b) {
  return (((a & 0xF) + (b & 0xF)) & 0x10) == 0x10;
}
bool CPU::getHalfCarryFlag(const dword a, const dword b) {
  return (((a & 0xFFF) + (b & 0xFFF)) & 0x1000) == 0x1000;
}

}
