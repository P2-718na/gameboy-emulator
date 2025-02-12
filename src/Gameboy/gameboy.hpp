#ifndef GAMEBOY_H
#define GAMEBOY_H
#include <vector>
#include "PPU.hpp"
#include "address-bus.hpp"
#include "cpu.hpp"
#include "timer-controller.hpp"


namespace gb {

class Cartridge;

class Gameboy {
public:
  typedef std::array<PPU::color, PPU::height_ * PPU::width_> ScreenBuffer;
private:

 // Friend classes are the ones that can request interrupts
  friend class PPU;
  friend class TimerController;

  // Internal components
  Cartridge* cart;
  AddressBus bus{};
  PPU ppu{this, &bus };
  CPU cpu{this, &bus };
  TimerController timers{this, &bus };
  ScreenBuffer screenBuffer{};
  std::string serialBuffer;

  // Handlers
  void requestInterrupt(FlagInterrupt interrupt);

public:
  // Constructor ///////////////////////////////////////////////////////////////
  explicit Gameboy(const std::vector<word>& rom);
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
