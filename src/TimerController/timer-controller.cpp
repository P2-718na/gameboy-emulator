#include <cassert>

#include "timer-controller.hpp"
#include "gameboy.hpp"
#include "address-bus.hpp"

namespace gb {
// 8 here is needed because of the ugly hack I am using
std::array<int, 8> TimerController::timaRates{};

void TimerController::incrementTimer(TimerAddress timer) {
  assert(
    (timer == DividerRegister || timer == TIMARegister)
    && "Only timers can be incremented this way."
  );
  const auto oldValue = bus->read(timer);
  const bool overflow = oldValue == 0xFF;

  if (!overflow || timer == DividerRegister) {
    bus->write(timer, oldValue + 1);
    return;
  }

  // If TMA register overflows...
  const auto TMA = bus->read(TMARegister);
  bus->write(TIMARegister, TMA);
  gameboy->requestInterrupt(TimerBit);
}

// Public ////////////////////////////////////////////
TimerController::TimerController(Gameboy* gameboy, AddressBus* ram)
  : GBComponent{gameboy, ram}
{
  // Possible TIMA rates in machine cycles.
  // Todo this is a bit ugly, please fix
  timaRates[0b111] = 64;
  timaRates[0b110] = 16;
  timaRates[0b101] = 4;
  timaRates[0b100] = 256;
}

void TimerController::machineClock() {
  ++clockCount;
  // This function needs to be called once each machine clock.
  // Machine clock runs at 1'048'576 Hz.

  if (clockCount % divTimerRate == 0) {
    // Here overflow does not trigger an interrupt
    incrementTimer(DividerRegister);
  }

  const std::bitset<3> TAC = bus->read(TACRegister);
  if (!TAC[2]) {
    // This timer is not enabled
    return;
  }

  const auto timaTimerRate = timaRates[TAC.to_ulong()];
  assert(timaTimerRate != 0);

  if (clockCount % timaTimerRate == 0) {
    // here overflow triggers an interrupt.
    // This should probably be handled by bus but for now it is handled
    // in incrementTimer
    incrementTimer(TIMARegister);
  }
}

}