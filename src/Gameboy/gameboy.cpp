#include <fstream>
#include <iterator>
#include "gameboy.hpp"
#include "processor.hpp"
#include "memory.hpp"
#include <cassert>

namespace gb {
// This is used in case I want to have different cpu implementations
// and/or savestates.
Gameboy::Gameboy(Processor& cpu, Memory& ram, Graphics& ppu, const std::string& romPath)
  : ppu_(ppu)
  , cpu_(cpu)
  , ram_(ram) {
    std::ifstream input(romPath, std::ios_base::binary);

    if(input.fail()){
      printf("Error reading ROM file.");
      exit(1);
    }

    // copies all data into buffer
    rom_ = std::vector<word>(std::istreambuf_iterator<char>(input), {});

    assert(rom_.size() % 0x4000 == 0);

    ram_.setBank0(rom_);

  // todo check cartridge

    // TODO this is very hacky, obv need to change
    ram_.setBank1(rom_);

  ppu_.doTheUglyHackyThing(&cpu_);
}

void Gameboy::clock() {
  cpu_.machineClock();
  ppu_.machineClock();
}

} // namespace gb