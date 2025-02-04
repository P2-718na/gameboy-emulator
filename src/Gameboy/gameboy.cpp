#include "gameboy.hpp"

#include "processor.hpp"
#include "memory.hpp"

namespace gb {
// This is used in case I want to have different cpu implementations
// and/or savestates.
Gameboy::Gameboy(Processor& cpu, Memory& ram, Graphics& ppu) : cpu_(cpu), ram_(ram), ppu_(ppu) {

}

void Gameboy::clock() {
  cpu_.machineClock();
  ppu_.machineClock();
}

} // namespace gb