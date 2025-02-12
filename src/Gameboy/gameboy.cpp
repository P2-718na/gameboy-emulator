#include "gameboy.hpp"
#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include "address-bus.hpp"
#include "cpu.hpp"

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
  timers.machineClock();
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