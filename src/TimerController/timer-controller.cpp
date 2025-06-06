#include <cassert>

#include "timer-controller.hpp"
#include "gameboy.hpp"
#include "address-bus.hpp"

namespace gb {

constexpr std::array<int, 4> TimerController::TIMA_RATES;

void TimerController::incrementTimer(dword address) {
  assert(
    (address == REG_DIV || address == REG_TIMA)
    && "Only timers can be incremented this way."
  );

  // First, check for overflow.
  const auto oldValue = bus->read(address);
  const bool overflow = oldValue == 0xFF;

  // DIV timer does not trigger an interrupt on overflow.
  // TIMA triggers an interrupt ONLY when it overflows.
  if (address == REG_DIV || !overflow) {
    bus->write(address, oldValue + 1, AddressBus::TC);
    return;
  }

  // If TMA register overflows, it gets reset to TMA value.
  const auto TMA = bus->read(REG_TMA);
  bus->write(REG_TIMA, TMA, AddressBus::TC);
  gameboy->requestInterrupt(INTERRUPT_TIMER);
}

// Public ////////////////////////////////////////////
TimerController::TimerController(Gameboy* gameboy, AddressBus* bus)
: bus{ bus }
, gameboy{ gameboy }
{}

void TimerController::machineClock() {
  ++clockCount;


  if (clockCount % DIV_RATE == 0) {
    // Here overflow does not trigger an interrupt
    incrementTimer(REG_DIV);
  }

  const std::bitset<3> TAC = bus->read(REG_TAC);
  // This bit indicates wether timer is enabled or not.
  if (!TAC[2]) {
    return;
  }

  // Just the two lower bits of the register are used to select rate.
  const auto TIMARateSelector = TAC.to_ulong() & 0b11;
  const auto currentTIMARate = TIMA_RATES[TIMARateSelector];
  assert(currentTIMARate != 0 && "Tima rates have not been initialized properly.");

  if (clockCount % currentTIMARate == 0) {
    // here overflow triggers an interrupt.
    // This should probably be handled by bus but for now it is handled
    // in incrementTimer
    incrementTimer(REG_TIMA);
  }
}

}