#include "address-bus.hpp"
#include "types.hpp"
#include "cartridge.hpp"

#include <cassert>
#include <gameboy.hpp>
#include <iostream>
#include <vector>

namespace gb {

constexpr std::array<word, 0x100> AddressBus::BOOT_ROM;

bool AddressBus::isBootRomEnabled() const {
  // 1 = disabled, 0 = enabled
  return (memory[BOOT_ROM_LOCK] & 0b1) == 0b0;
}

bool AddressBus::isCartridgeInserted() const {
  return cart != nullptr;
}

bool AddressBus::refersToCartridge(gb::dword address) {
  return (address < 0x8000) || (address >= 0xA000 && address < 0xC000);
}

AddressBus::AddressBus(Gameboy* gameboy) : gameboy{gameboy} {
  assert(sizeof(word) == 1);
  assert(sizeof(dword) == 2);
}

// Todo smart pointers
void AddressBus::loadCart(Cartridge* newCart) {
  cart = newCart;
};

word AddressBus::read(const dword address) {
  if (address < 0x100u && isBootRomEnabled()) {
    return BOOT_ROM[address];
  }

  if (!refersToCartridge(address)) {
    return memory[address];
  }

  if (!isCartridgeInserted()) {
    return 0xFF;
  }

  return cart->read(address);
}

// Todo maybe
//  I want another function "writeRegister" that handles writing to specific
//  registers
void AddressBus::write(const dword address, const word value, Component whois) {

  if (refersToCartridge(address)) {
    assert(isCartridgeInserted() && "Trying to write to Cartridge without any inserted. This should not be possible, as boot rom does not perform write operations.");
    cart->write(address, value);
    return;
  }

  // Special registers
  if (address == 0xFF41) {
    // The two lower bits are only writable by PPU!
    const word mask = (whois == Ppu ? 0b00000011 : 0b11111100);
    memory[address] &= ~mask;
    memory[address] |= value & mask;
    return;
  }

  // FF04 — DIV: Divider register
  if (address == 0xFF04) {
    memory[address] = 0x00;
    return;
  }

  memory[address] = value;

  // Serial communication
  // todo add some proper interface
  if (address == 0xFF02 && value == 0x81) {
    gameboy->serialBuffer += read(0xFF01);
    return;
  }

  // Some edge cases in the book:
  if (address >= 0xE000 && address <= 0xFE00) {
    memory[address - 0x2000] = value;
    return;
  }

  if (address >= 0xC000 && address <= 0xDE00) {
    memory[address + 0x2000] = value;
    return;
  }
}


}