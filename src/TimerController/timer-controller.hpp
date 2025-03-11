#ifndef TIMER_CONTROLLER_H
#define TIMER_CONTROLLER_H
#include <array>

#include "types.hpp"

namespace gb {

class Gameboy;
class AddressBus;

class TimerController {
  // Bare pointers are not ideal; see Gameboy
  AddressBus* bus;
  Gameboy* gameboy;

  // TIMA timer can be incremented with four different rates (chosen by the ROM code).
  // These are stored in the 2 lower bits of  TAC (Timer access control) register.
  // Depending on those bits, this is the rate of the timer.
  static constexpr std::array<int, 4> TIMA_RATES{ 256, 4, 16, 64 };

  // DIV timer increments at a fixed rate of once every 64 machine clocks.
  static constexpr int DIV_RATE = 64;

  // Emulator "fake" variable to check when timer triggers. In real GB this would have
  // been a physical signal that originated from a hardware counter.
  long long unsigned clockCount{0};

  // Increments a specific timer. This takes care of interrupt calling and of timer-specific
  // additional logic.
  void incrementTimer(dword address);

 public:
  TimerController(Gameboy* gameboy, AddressBus* bus);

  // This function needs to be called once each machine clock.
  // Machine clock runs at 1'048'576 Hz.
  void machineClock();
};

}

#endif  // TIMER_CONTROLLER_H
