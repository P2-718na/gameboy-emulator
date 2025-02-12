#include "address-bus.hpp"
#include "types.hpp"

#include <cassert>
#include <vector>
#include <iostream>

namespace gb {

constexpr std::array<word, 0x100> AddressBus::bootRom_;

bool AddressBus::isBootRomEnabled() {
  // 1 = disabled, 0 = enabled
  return (bus[BOOT_ROM_LOCK] & 0b1) == 0b0;
}

AddressBus::AddressBus() {
  assert(sizeof(word) == 1);
  assert(sizeof(dword) == 2);
};

word AddressBus::read(const dword address) {
  if (address < 0x100u && isBootRomEnabled()) {
    return bootRom_[address];
  }

  return bus[address];
}

// Todo maybe
//  I want another function "writeRegister" that handles writing to specific
//  registers
void AddressBus::write(const addr address, const word value, Component whois) {
  // Special registers
  if (address == 0xFF41 && whois != Ppu) {
    assert(false && "Someone tried to write to read-only STAT register!");
    return;  // Todo maybe handle proper edge case
  }

  // FF04 — DIV: Divider register
  if (address == 0xFF04) {
    bus[address] = 0x00;
    return;
  }

  if (address < 0x8000) {
    printf("%04x\n", address);
    assert(false && "Someone tried to write to read-only ROM!");
    return;  // Todo maybe handle proper edge case
  }

  bus[address] = value;

  // Serial communication
  // todo add some proper interface
  if (address == 0xFF02 && value == 0x81) {
    printf("%c", read(0xFF01));
  }

  // Some edge cases in the book:
  if (address >= 0xE000 && address <= 0xFE00) {
    bus[address - 0x2000] = value;
  }

  if (address >= 0xC000 && address <= 0xDE00) {
    bus[address + 0x2000] = value;
  }
}

void AddressBus::setBank0(const std::vector<word>& rom) {
  //todo const
  std::copy(rom.begin(), rom.begin()+0x4000, bus.begin());
}

void AddressBus::setBank1(const std::vector<word>& rom) {
  //todo const
  std::copy(rom.begin()+0x4000, rom.begin()+0x8000, bus.begin()+0x4000);
}

void AddressBus::printROM() {
  for (int i = 0; i != 0x4000; ++i) {
    if (i % 0x100 == 0) {
      printf("\n");
    }
    printf("%02x ", bus[i]);
  }
}


}