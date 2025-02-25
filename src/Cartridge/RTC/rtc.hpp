//
// Created by Matteo Bonacini on 12/02/25.
//

// TODO move this to another branch

#ifndef RTC_H
#define RTC_H

#include <array>
#include <chrono>
#include <cassert>

#include "types.hpp"

namespace gb {

using namespace std::chrono;

struct RTC {
  // TODO add proper implementation of halt and carry bit
  bool haltBit{false};
  bool carryBit{false};

  // TODO implement save and load of latched registers
  std::array<word, 5> regs;

  inline word rtS() const {
    const auto clock = system_clock::now();
    return duration_cast<seconds>(clock.time_since_epoch()).count() % 60;
  }

  inline word rtM() const {
    const auto clock = system_clock::now();
    return duration_cast<minutes>(clock.time_since_epoch()).count() % 60;
  }

  inline word rtH() const {
    const auto clock = system_clock::now();
    return duration_cast<hours>(clock.time_since_epoch()).count() % 24;
  }

  // Day register is 9-bit
  inline word rtDL() const {
    const auto clock = system_clock::now();
    const auto hoursPassed = duration_cast<hours>(clock.time_since_epoch()).count();
    const auto daysPassed = hoursPassed / 24;
    return (daysPassed % 0b111111111) & 0xFF;
  }
  inline word rtDH() const {
    const auto clock = system_clock::now();
    const auto hoursPassed = duration_cast<hours>(clock.time_since_epoch()).count();
    const auto daysPassed = hoursPassed / 24;
    const unsigned int result = (daysPassed % 0b111111111) & 0b100000000;
    return result & (haltBit << 6) & (carryBit << 7);
  }

  inline void latch() {
    regs[0] = rtS();
    regs[1] = rtM();
    regs[2] = rtH();
    regs[3] = rtDL();
    regs[4] = rtDH();
  }

  inline word getLatchedRegister(word reg) const {
    assert(reg >= 0x08 && reg < 0x0D);
    return regs[reg - 0x08];
  }

  // Todo this should also write to realtime registers.
  inline void writeRegister(word reg, word value) {
    const int map = reg - 0x08;

    if (map == 4) {
      regs[map] = value & 0b11000001;
      return;
    }

    if (map == 3) {
      regs[map] = value & 0b11111111;
      return;
    }

    regs[map] = value & 0b00111111;
  }
};

}

#endif  // RTC_H
