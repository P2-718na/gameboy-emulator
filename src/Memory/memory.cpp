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

  return memory_[address];
}

// Todo maybe
//  I want another function "writeRegister" that handles writing to specific
//  registers
void Memory::write(const addr address, const word value, Component whois) {
  // Special registers
  if (address == 0xFF41 && whois != Ppu) {
    assert(false && "Someone tried to write to read-only STAT register!");
    return;  // Todo maybe handle proper edge case
  }

  // FF04 — DIV: Divider register
  if (address == 0xFF04) {
    memory_[address] = 0x00;
    return;
  }

  if (address < 0x8000) {
    printf("%04x\n", address);
    assert(false && "Someone tried to write to read-only ROM!");
    return;  // Todo maybe handle proper edge case
  }

  memory_[address] = value;

  // Serial communication
  // todo add some proper interface
  if (address == 0xFF02 && value == 0x81) {
    printf("%c", read(0xFF01));
  }

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

void Memory::setBank1(const std::vector<word>& rom) {
  //todo const
  std::copy(rom.begin()+0x4000, rom.begin()+0x8000, memory_.begin()+0x4000);
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