#ifndef TIMER_CONTROLLER_H
#define TIMER_CONTROLLER_H

#include "types.hpp"

namespace gb {

class Gameboy;
class AddressBus;

class TimerController {
  static std::array<int, 8> timaRates;

  // Todo all of these should probably be defined in addressBus
  static constexpr dword TMARegister = 0xFF06;
  static constexpr dword TACRegister = 0xFF07;
  typedef enum : dword {
    DividerRegister = 0xFF04,
    TIMARegister    = 0xFF05
  } TimerAddress;

  // Todo prob use shared ptr
  AddressBus* ram;
  Gameboy* gameboy;

  // Emulator "fake" variable to check when timer triggers
  long long unsigned clockCount{0};

  void incrementTimer(TimerAddress timer);

 public:
  TimerController(Gameboy* gameboy, AddressBus* ram);

  void machineClock();
};

}

#endif  // TIMER_CONTROLLER_H
