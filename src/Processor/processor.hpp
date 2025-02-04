#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <bitset>

#include "types.hpp"
#include "memory.hpp"

namespace gb {

class Processor {

  bool breakpoint_ = false;
  bool interrupt_  = false;

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

  dword BC();
  void BC(word msb, word lsb);
  void BC(dword value);
  dword DE();
  void DE(word msb, word lsb);
  void DE(dword value);
  dword HL();
  void HL(word msb, word lsb);
  void HL(dword value);

  void incrementRegister(word& reg);
  void decrementRegister(word& reg);
  void subRegister(word& reg);
  void cmpRegister(word reg);

  void setSP(word msb, word lsb);
  void setPC(word msb, word lsb);

  word popPC();

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

  // Fixme I don't like this. It would be nice to have a reference... Or to refactor the code in some way
  Memory* ram_;

  int busyCycles{ 0 };

  static int getBusyCycles(Opcode opcode);

  static int getBusyCyclesCB(CBOpcode opcode);

  static bool nthBit(word byte, int bit);

  void executeOpcode(Opcode opcode);

  void executeCBOpcode(CBOpcode opcode);

 public:
    Processor();

    // Todo I dont like this. I'd rather prefer that
    //  this class has a reference to gameboy and that it can call ram from that.
    void connectMemory(Memory* ram);

    void printRegisters();

    void printRegistersIfChanged();

    void machineClock();

    // Todo make private
    void executeCurrentInstruction();

    // Todo make private
    void handleInterrupts();

    void setInterrupt(dword address);

    bool breakpoint() const;


  ///////////////////////////////////////////////////////
    static dword twoWordToDword(word msb, word lsb);
    static word dwordMsb(dword value);
    static word dwordLsb(dword value);
};

}  // namespace gb

#endif //PROCESSOR_H
