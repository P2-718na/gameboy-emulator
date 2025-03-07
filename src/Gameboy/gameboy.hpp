#ifndef GAMEBOY_H
#define GAMEBOY_H
#include <string>
#include <vector>
#include "address-bus.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "timer-controller.hpp"

namespace gb {

class Cartridge;

/**
 * Main emulator class.
 * @author Matteo Bonacini "P2-718na"
 */
class Gameboy {
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
  TimerController tcu{this, &bus };

  // Status of the joypad. Here, I use low nibble for DIRECTIONAL controls
  // and high nibble to store BUTTONS. This is different from how the data
  // is stored/read from real hardware.
  // This variable is used only to check wether we should fire an interrupt or
  // not (that is, to check if the input status has changed between
  // setJoypad calls).
  word joypadStatus{0b11111111};

  // Depends on if the cartridge is battery backed.
  bool isCartridgeBatteryBacked;

  // "Asks" the CPU for an interrupt. Actually, most of the interrupt related
  // code could be moved inside Gameboy but, in hardware, interrupts are handled
  // by the CPU. So, it makes sense to keep all the code inside CPU and
  // expose here the requestInterrupt function so that all the other components
  // can request interrupts without having a direct reference to CPU.
  // You should see this function as if it was a "hardware interrupt request wire" inside
  // that connects all components inside the physical Game Boy.
  void requestInterrupt(INTERRUPT_ID interrupt);

public:
  // Constructor ///////////////////////////////////////////////////////////////
  explicit Gameboy(const Binary& rom);
  //////////////////////////////////////////////////////////////////////////////

  // These buffers could also be made read-only, but there is no effect in writing
  // to them.
  typedef std::array<PPU::color, PPU::height_ * PPU::width_> ScreenBuffer;
  ScreenBuffer screenBuffer{};
  std::string serialBuffer;

  void machineClock();
  void skipBoot();
  void setJoypad(word value);

  void loadSave(const Binary& ram);
  const Binary& getSave();
  bool shouldSave() const;

  // Original hardware could turn off display.
  bool isScreenOn() const;

  // Debug functions
  void printScreenBuffer() const;
  void printSerialBuffer();
};

} // namespace gb

#endif //GAMEBOY_H
