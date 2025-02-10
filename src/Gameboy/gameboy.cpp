#include <cassert>
#include <exception>
#include <fstream>
#include <iterator>
#include <iostream>
#include "gameboy.hpp"
#include "memory.hpp"
#include "processor.hpp"

namespace gb {

void Gameboy::requestInterrupt(FlagInterrupt interrupt) {
  cpu.IF(interrupt, true);
}

void Gameboy::setupROM(const std::string& romPath) {
  std::ifstream input(romPath, std::ios_base::binary);
  if (input.fail()) {
    throw std::runtime_error("Error reading ROM file!");
  }

  // Copy all data to ROM, it's faster than reading from file.
  rom = std::vector<word>(std::istreambuf_iterator<char>(input), {});

  // Todo move hardcoded const
  if (rom.size() % 0x4000 != 0) {
    throw std::runtime_error("Invalid ROM file: blocks must be multiples of 16KiB!");
  }

  ram.setBank0(rom);
  // todo check cartridge
  // TODO this is very hacky, obv need to change
  ram.setBank1(rom);
}

// TImer logic

void Gameboy::clockTimers() {
  // This function needs to be called once each machine clock.
  // Machine clock runs at 1'048'576 Hz.

  // Div timer gets updated at a rate of 16384Hz.
  constexpr auto divTimerRate = 16384;
  if (clockCount % divTimerRate == 0) {
    // Here overflow does not trigger an interrupt
    incrementTimer(0xFF04);
  }

  // 0xFF07 is the tac register
  // todo move hardcoded stuff elsewhere
  const std::bitset<3> TAC = ram.read(0xFF07);
  if (!TAC[2]) {
    // This timer is not enabled
    return;
  }

  const auto timaTimerRate = cpu.timaRates[TAC.to_ulong()];

  if (clockCount % timaTimerRate == 0) {
    // here overflow triggers an interrupt.
    // This should probably be handled by RAM but for now it is handled
    // in incrementTimer
    incrementTimer(0xFF05);
  }
}

void Gameboy::incrementTimer(dword address) {
  assert(("Only timers can be incremented this way.", address == 0xFF04 || address == 0xFF05));
  const auto oldValue = ram.read(address);
  const bool overflow = oldValue == 0xFF;

  if (overflow && address == 0xFF05) {
    //todo ^^ vv hardcoded stuff
    const auto TMA = ram.read(0xFF06);
    ram.write(address, TMA);
    requestInterrupt(TimerBit);
    return;
  }

  ram.write(address, oldValue + 1);
}

// Constructors /////////////////////
Gameboy::Gameboy(const std::string& romPath) {
  setupROM(romPath);
}

 Gameboy::Gameboy(State state) {
  // Todo implement this
  assert(false);
}



// Public /////////////////////////////////////////////////////////
void Gameboy::machineClock() {
  clockTimers();
  cpu.machineClock();
  ppu.machineClock();
}

// getters
const Gameboy::ScreenBuffer& Gameboy::getScreenBuffer() const {
  return screenBuffer;
}


bool Gameboy::isScreenOn() const {
  return ppu.LCDC(Graphics::LCD_Display_Enable);
}

// Debug
void Gameboy::printScreenBuffer() const {
  static constexpr std::array<char, 4> grayscale{'.', 'o', '#', '@'};

  for (int y = 0; y != 144; ++y) {
    for (int x = 0; x != 160; ++x) {
      const auto pixel = grayscale[screenBuffer[x + y * Graphics::width_].to_ulong()];
      std::cout << pixel;
    }

    std::cout << std::endl;
  }
}

void Gameboy::printSerialBuffer() const {
  std::cout << "\092[0m" << serialBuffer;
}


} // namespace gb