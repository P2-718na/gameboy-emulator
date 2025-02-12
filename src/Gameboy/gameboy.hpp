#ifndef GAMEBOY_H
#define GAMEBOY_H
#include <vector>
#include "address-bus.hpp"
#include "processor.hpp"
#include "graphics.hpp"
#include "timer-controller.hpp"

namespace gb {

class Gameboy {
public:
  typedef std::array<Graphics::color, Graphics::height_ * Graphics::width_> ScreenBuffer;
private:

 // Friend classes are the ones that can request interrupts
  friend class Graphics;
  friend class TimerController;

  // Internal components
  std::vector<word> rom{};
  AddressBus ram{};
  Graphics ppu{this, &ram};
  Processor cpu{this, &ram};
  TimerController timers{this, &ram};
  ScreenBuffer screenBuffer{};
  std::string serialBuffer;

  // Handlers
  void requestInterrupt(FlagInterrupt interrupt);

  // Init functions
  // Todo i don't think this makes sense, rom should already be loaded
  // in memory by frontend
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

  // Getters
  const ScreenBuffer& getScreenBuffer() const;
  bool isScreenOn() const;

  // Debug functions
  void printScreenBuffer() const;
  void printSerialBuffer() const;
};

} // namespace gb

#endif //GAMEBOY_H
