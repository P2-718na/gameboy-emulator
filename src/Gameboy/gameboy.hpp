#ifndef GAMEBOY_H
#define GAMEBOY_H
#include <vector>
#include <string>
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
  friend class AddressBus;
  friend class TimerController;

  // Internal components
  Cartridge* cart;
  AddressBus bus{this};
  PPU ppu{this, &bus };
  CPU cpu{this, &bus };
  TimerController timers{this, &bus };

  // Status of the joypad. Here, I use low nibble for DIRECTIONAL controls
  // and high nibble to store BUTTONS.
  word joypadStatus{0b11111111};

  // Handlers
  void requestInterrupt(FlagInterrupt interrupt);

public:
  // Constructor ///////////////////////////////////////////////////////////////
  explicit Gameboy(const std::vector<word>& rom);
 //////////////////////////////////////////////////////////////////////////////
  ScreenBuffer screenBuffer{};
  std::string serialBuffer;
  bool shouldSave;

  void machineClock();
  void skipBoot();
  void loadSave(const std::vector<word>& ram);
  const std::vector<word>& getSave();
  // Here value contains wether or not the inputs are pressed.
  // This should be called only on a rising/falling edge of one interrupt,
  // otherwise the call will be ignored and the interrupt not fired.
  // A bit of 1 means that the button is NOT pressed. value must contain on the
  // lower nibble the values for the D-PAD and in the higher nibble the values
  // for the buttons.
  void setJoypad(word value);

  // Getters
  bool isScreenOn() const;

  // Debug functions
  void printScreenBuffer() const;
  void printSerialBuffer();
};

} // namespace gb

#endif //GAMEBOY_H
