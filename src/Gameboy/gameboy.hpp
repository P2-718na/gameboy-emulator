#ifndef GAMEBOY_H
#define GAMEBOY_H
#include "memory.hpp"
#include "processor.hpp"

namespace gb {
class Gameboy {

public:
  // Constructor ///////////////////////////////////////////////////////////////
  Gameboy();
  Gameboy(Processor& cpu, Memory& ram);
  Gameboy(const Processor& cpu);
 //////////////////////////////////////////////////////////////////////////////

  void turnOn();
  Processor cpu_;
  Memory ram_;
};

} // namespace gb

#endif //GAMEBOY_H
