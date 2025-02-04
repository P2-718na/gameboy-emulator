#include "memory.hpp"
#include "types.hpp"

#include <cassert>

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

}