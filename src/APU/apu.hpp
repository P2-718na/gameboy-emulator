#ifndef APU_H
#define APU_H

#include <bitset>

#include "memory.hpp"

namespace gb {

// Todo rename this with PPU or somehting
class APU {

  //  Todo same as for processor.hpp
  Memory* ram_;

 public:
  APU();

  // Todo every element that has some "machine clock" function should be derived class
  // of another class with pure function machineClock() along with ram_ stuff
  void machineClock();
};

} // namespace gb

#endif //APU_H
