#ifndef GAMEBOY_H
#define GAMEBOY_H
#include <vector>
#include "memory.hpp"
#include "processor.hpp"
#include "graphics.hpp"

namespace gb {

class Gameboy {
public:
  typedef std::array<Graphics::color, Graphics::height_ * Graphics::width_> ScreenBuffer;
private:

  friend class Graphics;

  // Internal components
  std::vector<word> rom{};
  Memory ram{};
  Graphics ppu{this, &ram};
  Processor cpu{this, &ram};
  ScreenBuffer screenBuffer{};
  std::string serialBuffer;

  // Emulator "fake" variables
  long long unsigned clockCount{0};

  // Handlers
  void requestInterrupt(FlagInterrupt interrupt);

  // Init functions
  void setupComponents();
  void setupROM(const std::string& romPath);

  // Timer handlers
  // TOdo these should probably have their own class
  void clockTimers();
  void incrementTimer(dword address);

public:
  struct State {
    // TODO use this to implement savestate loading
  };

  // Constructor ///////////////////////////////////////////////////////////////
  explicit Gameboy(const std::string& romPath);
  explicit Gameboy(State state);
 //////////////////////////////////////////////////////////////////////////////

  void machineClock();

  // Getters
  const ScreenBuffer& getScreenBuffer() const;
  bool isScreenOn() const;

  // Debug functions
  void printScreenBuffer() const;
  void printSerialBuffer() const;
};

} // namespace gb

#endif //GAMEBOY_H
