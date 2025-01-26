#include "gameboy.hpp"

#include "processor.hpp"
#include "memory.hpp"

namespace gb {
Gameboy::Gameboy() = default;

// This is used in case I want to have different cpu implementations
// and/or savestates.
Gameboy::Gameboy(Processor& cpu, Memory& ram) : cpu_(cpu), ram_(ram) {
  cpu_.connectMemory(&ram_);
}

void Gameboy::clock() {
  cpu_.machineClock();
  ppu_.machineClock();
}


void Gameboy::turnOn() {

}


} // namespace gb