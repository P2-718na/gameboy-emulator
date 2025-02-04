#ifndef GAMEBOY_H
#define GAMEBOY_H
#include <vector>
#include "memory.hpp"
#include "processor.hpp"
#include "graphics.hpp"

namespace gb {
class Gameboy {
  std::vector<word> rom_{};

public:
  // Constructor ///////////////////////////////////////////////////////////////
  Gameboy(Processor& cpu, Memory& ram, Graphics& ppu);
  Gameboy(Processor& cpu, Memory& ram, Graphics& ppu, const std::string& romPath);
 //////////////////////////////////////////////////////////////////////////////

  void clock();

  // This is only needed in case we want multithreading
  //void turnOn();

  Graphics& ppu_;
  Processor& cpu_;
  Memory& ram_;
};

} // namespace gb

#endif //GAMEBOY_H
