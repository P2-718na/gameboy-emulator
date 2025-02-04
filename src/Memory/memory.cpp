#include "memory.hpp"
#include "types.hpp"

#include <cassert>
#include <vector>
#include <iostream>

namespace gb {

bool Memory::isBootRomEnabled() {
  // 1 = disabled, 0 = enabled
  return (memory_[BOOT_ROM_LOCK] & 0b1) == 0b0;
}

 Memory::Memory() {
  assert(sizeof(word) == 1);
  assert(sizeof(addr) == 2);
};

word Memory::read(const addr address) {
  if (address <= 0x100 && isBootRomEnabled()) {
    return bootRom_[address];
  }

    // Todo remove this is just for debug
    if (address <= 0x133 && address >= 0x104) {

    }

    return memory_[address];
  }


void Memory::write(const addr address, const word value) {

    memory_[address] = value;

    // Some edge cases in the book:
    if (address >= 0xE000 && address <= 0xFE00) {
      memory_[address - 0x2000] = value;
    }

    if (address >= 0xC000 && address <= 0xDE00) {
      memory_[address + 0x2000] = value;
    }
}

void Memory::setBank0(const std::vector<word>& rom) {
  //todo const
  std::copy(rom.begin(), rom.begin()+0x4000, memory_.begin());
}

void Memory::printROM() {
  for (int i = 0; i != 0x4000; ++i) {
    if (i % 0x100 == 0) {
      printf("\n");
    }
    printf("%02x ", memory_[i]);
  }
}


}