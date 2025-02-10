#ifndef GAMEBOY_H
#define GAMEBOY_H
#include <vector>
#include "memory.hpp"
#include "processor.hpp"
#include "graphics.hpp"

namespace gb {

class Gameboy {

  std::vector<word> rom{};
  Memory ram{};
  Graphics ppu{this, &ram};
  Processor cpu{this, &ram};

  // Init functions
  void setupComponents();
  void setupROM(const std::string& romPath);

public:
  struct State {
    // TODO use this to implement savestate loading
  };

  // Constructor ///////////////////////////////////////////////////////////////
  explicit Gameboy(const std::string& romPath);
  explicit Gameboy(State state);
 //////////////////////////////////////////////////////////////////////////////

  void machineClock();
};

} // namespace gb

#endif //GAMEBOY_H
