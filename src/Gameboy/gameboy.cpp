#include <cassert>
#include <exception>
#include <fstream>
#include <iterator>
#include "gameboy.hpp"
#include "memory.hpp"
#include "processor.hpp"

namespace gb {

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

Gameboy::Gameboy(const std::string& romPath) {
  setupROM(romPath);
}

 Gameboy::Gameboy(State state) {
  // Todo implement this
  assert(false);
}

// Public /////////////////////////////////////////////////////////
void Gameboy::machineClock() {
  cpu.machineClock();
  ppu.machineClock();
}

} // namespace gb