#include "timer-controller.hpp"
#include "gameboy.hpp"
#include "memory.hpp"

namespace gb {
// 8 here is needed because of the ugly hack I am using
std::array<int, 8> TimerController::timaRates{};

void TimerController::incrementTimer(TimerAddress timer) {
  assert(
    ("Only timers can be incremented this way.",
     timer == DividerRegister || timer == TIMARegister));
  const auto oldValue = ram->read(timer);
  const bool overflow = oldValue == 0xFF;

  if (overflow && timer == TIMARegister) {
    // todo ^^ vv hardcoded stuff
    const auto TMA = ram->read(TMARegister);
    ram->write(TIMARegister, TMA);
    gameboy->requestInterrupt(TimerBit);
    return;
  }

  ram->write(timer, oldValue + 1);
}

// Public ////////////////////////////////////////////
TimerController::TimerController(Gameboy* gameboy, Memory* ram)
  : ram{ ram }
  , gameboy{ gameboy }
{
  // Possible TIMA rates in machine cycles.
  // Todo this is a bit ugly, please fix
  timaRates[0b111] = 16384 ;
  timaRates[0b110] = 65536 / 4;
  timaRates[0b101] = 262144 / 4;
  timaRates[0b100] = 4096 / 4;
}

void TimerController::machineClock() {
  // This function needs to be called once each machine clock.
  // Machine clock runs at 1'048'576 Hz.

  // Div timer gets updated at a rate of 16384Hz.
  constexpr auto divTimerRate = 16384;
  if (clockCount % divTimerRate == 0) {
    // Here overflow does not trigger an interrupt
    incrementTimer(DividerRegister);
  }

  // 0xFF07 is the tac register
  // todo move hardcoded stuff elsewhere
  const std::bitset<3> TAC = ram->read(TACRegister);
  if (!TAC[2]) {
    // This timer is not enabled
    return;
  }

  const auto timaTimerRate = timaRates[TAC.to_ulong()];
  assert(timaTimerRate != 0);

  if (clockCount % timaTimerRate == 0) {
    // here overflow triggers an interrupt.
    // This should probably be handled by RAM but for now it is handled
    // in incrementTimer
    incrementTimer(TIMARegister);
  }
}

}