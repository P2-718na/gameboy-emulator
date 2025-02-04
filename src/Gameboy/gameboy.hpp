#ifndef GAMEBOY_H
#define GAMEBOY_H
#include "memory.hpp"
#include "processor.hpp"
#include "graphics.hpp"

namespace gb {
class Gameboy {

public:
  // Constructor ///////////////////////////////////////////////////////////////
 Gameboy(Processor& cpu, Memory& ram, Graphics& ppu);
 //////////////////////////////////////////////////////////////////////////////

  void clock();

  void turnOn();

  Graphics ppu_;
  Processor cpu_;
  Memory ram_;
};

} // namespace gb

#endif //GAMEBOY_H
