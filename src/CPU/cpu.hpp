#ifndef CPU_H
#define CPU_H

#include <bitset>
#include <array>

#include "types.hpp"

namespace gb {

class Gameboy;
class AddressBus;

class CPU {
  // Gameboy needs to be called the private IF method (used to request interrupts).
  // IF() COULD be moved inside Gameboy, but from a hardware point of view
  // interrupts are handled by the CPU, so it makes sense to have most of the interrupt-related code here.
  friend class Gameboy;

  // Bare pointers are not ideal but still fine; see Gameboy
  AddressBus* bus;
  Gameboy* gameboy;

  // Instruction timings in machine cycles.
  // For clarity's sake, the values are populated in the constructor.
  // All the timing constants are defined in the "timings.hpp" file.
  static std::array<int, 256> INSTRUCTION_TIMINGS;
  static std::array<int, 256> CB_INSTRUCTION_TIMINGS;
  static void initTimings();
  static void initTimingsCB();
  // Interrupts make execution jump to specific addresses. This array is populated in constructor.
  static std::array<dword, 5> INTERRUPT_JUMP_ADDRESSES;

  // "Special" conditions that stop CPU execution.
  bool halted{false};
  bool crashed{false};
  // When > 0, it indicates that the processor is in the process of executing an instruction.
  // This is actually just an approximation as all instructions are treated as atomic.
  int busyCycles{ 0 };

  // CPU internal registers. Some pairs of 1-word registers are sometimes use as a
  // 1-dword register (AF, BC, DE, HL; MSB is the leftmost register in the name).
  word           A{};
  std::bitset<8> F{}; // Stores results of some math operations
  word           B{};
  word           C{};
  word           D{};
  word           E{};
  word           H{};
  word           L{};
  dword          SP{}; // Stack pointer
  dword          PC{}; // Program counter
  bool           IME{false}; // "Interrupt master enable"

  // F register holds some flags at the following bit positions:
  typedef enum {
    FZ = 7,
    FN = 6,
    FH = 5,
    FC = 4
  } FLAG_BIT;
  // F register:
  //  Z N H C 0 0 0 0
  // Zero Flag (Z):
  //  This bit is set when the result of a math operation
  //  is zero or two values match when using the CP
  //  instruction.
  // Subtract Flag (N):
  //  This bit is set if a subtraction was performed in the
  //  last math instruction.
  // Half Carry Flag (H):
  //  This bit is set if a carry occurred from the lower
  //  nibble in the last math operation.
  // Carry Flag (C):
  //  This bit is set if a carry occurred from the last
  //  math operation or if register A is the smaller value
  //  when executing the CP instruction.

  // IF = Interrupt Flag register. If this register is != 0, it means that an
  // interrupt is requested. If multiple interrupts are requested,
  // they will be handled in order of importance (INTERRUPT_ID value)
  void IF(INTERRUPT_ID interrupt, bool enabled);
  std::bitset<5> IF() const;
  // Interrupt enable register. Holds information about which interrupts are enabled.
  std::bitset<5> IE() const;

  // Combined registers getters and setters
  dword BC() const;
  void  BC(word msb, word lsb);
  void  BC(dword value);
  dword DE() const;
  void  DE(word msb, word lsb);
  void  DE(dword value);
  dword HL() const;
  void  HL(word msb, word lsb);
  void  HL(dword value);

  // iHL = 1-word register at address given by HL register.
  word  iHL() const;
  void  iHL(word value);

  // Handlers for common operations ////////////////////////////////////////////
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

  // CPU main loop steps ///////////////////////////////////////////////////////
  void executeCurrentInstruction();
  void executeOpcode(OPCODE opcode);
  void executeCBOpcode(CB_OPCODE opcode);
  // tryTriggerInterrupts checks if it is possible to trigger an interrupt. If
  // yes, triggers it and returns true. otherwise, returns false.
  bool tryTriggerInterrupt();
  void triggerInterrupt(INTERRUPT_ID interrupt);

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  CPU(Gameboy* gameboy, AddressBus* bus);
  //////////////////////////////////////////////////////////////////////////////

  // Sets CPU state to after BOOT ROM execution.
  void reset();

  // Debug functions.
  void printRegisters() const;
  // this print is effectively delayed by one instruction
  void printRegistersIfChanged() const;
  // Used for testing to check that reset() succesfully resets program counter to 0x100.
  dword getPC() const;
  // Check if the current execution has stopped (busyCycles == 0).
  bool isBusy() const;

  // To be called once every machine clock.
  void machineClock();

  // Static methods ////////////////////////////////////////////////////////////
  // Convert back and from word<->dword
  static dword twoWordToDword(word msb, word lsb);
  static word  dwordMsb(dword value);
  static word  dwordLsb(dword value);

  // get nth bit of word
  static bool nthBit(word byte, int bit);

  // get number of cycles an instruction is supposed to take.
  static int  getBusyCycles(OPCODE opcode);
  static int  getBusyCyclesCB(CB_OPCODE opcode);

  // These only work for addition, not subtraction.
  // (Documentation about carry flags is kinda bad)
  static bool getCarryFlag(word a, word b);
  static bool getCarryFlag(dword a, dword b);
  static bool getHalfCarryFlag(word a, word b);
  static bool getHalfCarryFlag(dword a, dword b);
};

}  // namespace gb

#endif // CPU_H
