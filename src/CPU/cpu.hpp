#ifndef CPU_H
#define CPU_H

#include <bitset>
#include <array>

#include "types.hpp"
#include "gb-component.hpp"

namespace gb {

class Gameboy;
class AddressBus;

class CPU : public GBComponent {
  friend class Gameboy;
  // Todo flag should probably be moved to ram
  // also ram should probably be renamed something like
  // "Address bus"
  void IF(FlagInterrupt interrupt, bool enabled);

  static std::array<int, 256> timings_;
  static std::array<int, 256> timingsCB_;

  bool breakpoint_{false};
  bool halted_{false};
  static std::array<dword, 5> interruptAddresses;

  // MSB
  // LSB
  word A{};
  std::bitset<8> F{}; // Stores results of some math operations
  word B{};
  word C{};
  word D{};
  word E{};
  word H{};
  word L{};
  dword SP{}; // Stack pointer
  dword PC{}; // Program counter
  bool IME{false};

  // TODO move these to memory
  std::bitset<5> IE() const;
  std::bitset<5> IF() const;

  dword BC() const;
  void BC(word msb, word lsb);
  void BC(dword value);
  dword DE() const;
  void DE(word msb, word lsb);
  void DE(dword value);
  dword HL() const;
  void HL(word msb, word lsb);
  void HL(dword value);
  word iHL() const;
  void iHL(word value);

  static void initTimings();
  static void initTimingsCB();

  void incRegister(word& reg);
  void decRegister(word& reg);
  void addRegister(word reg);
  void subRegister(word reg);
  void andRegister(word reg);
  void  orRegister(word reg);
  void adcRegister(word reg);
  void sbcRegister(word reg);
  void xorRegister(word reg);
  void  cpRegister(word reg);

  void setSP(word msb, word lsb);
  void setPC(word msb, word lsb);
  void ret(bool condition);
  void jr(bool condition);
  void jpImm(bool condition);
  void callImm(bool condition);
  void loadImm(word& reg);

  word popPC();
  word popSP();
  signed char popPCSigned();
  void pushPCToStack();

  typedef enum {
    FZ = 7,
    FN = 6,
    FH = 5,
    FC = 4
  } FlagBit;
  // F register:
  // Z N H C 0 0 0 0
  // Zero Flag (Z):
  // This bit is set when the result of a math operation
  // is zero or two values match when using the CP
  // instruction.
  // ï Subtract Flag (N):
  // This bit is set if a subtraction was performed in the
  // last math instruction.
  // ï Half Carry Flag (H):
  // This bit is set if a carry occurred from the lower
  // nibble in the last math operation.
  // ï Carry Flag (C):
  // This bit is set if a carry occurred from the last
  // math operation or if register A is the smaller value
  // when executing the CP instruction.

  int busyCycles{ 0 };

  static int getBusyCycles(Opcode opcode);

  static int getBusyCyclesCB(CBOpcode opcode);

  static bool nthBit(word byte, int bit);

  void executeOpcode(Opcode opcode);

  void executeCBOpcode(CBOpcode opcode);

 public:
  CPU(Gameboy* gameboy, AddressBus* ram);

  void printRegisters();

  void printRegistersIfChanged();

  void machineClock() override;

  static void crash();

  // Todo make private
  void executeCurrentInstruction();

  // Todo make private
  bool handleInterrupts();
  void triggerInterrupt(FlagInterrupt interrupt);

  bool breakpoint() const;


///////////////////////////////////////////////////////
  static dword twoWordToDword(word msb, word lsb);
  static word dwordMsb(dword value);
  static word dwordLsb(dword value);

  static bool getCarryFlag(word a, word b);
  static bool getCarryFlag(dword a, dword b);
  static bool getHalfCarryFlag(word a, word b);
  static bool getHalfCarryFlag(dword a, dword b);
};
}  // namespace gb

#endif // CPU_H
