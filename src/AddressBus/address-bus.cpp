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

word AddressBus::getJoypad() const {
  const word joypadStatus = gameboy->joypadStatus;
  const word JOIP = memory[0xFF00];
  const std::bitset<6> joypadSelect = JOIP;
  constexpr word bitmaskHigh = 0b11110000;
  constexpr word bitmaskLow  = 0b00001111;

  // Select buttons
  if (!joypadSelect[5]) {
    return (JOIP & bitmaskHigh) | (joypadStatus >> 4);
  }

  // Select D-PAD
  if (!joypadSelect[4]) {
    return (JOIP & bitmaskHigh) | (joypadStatus & bitmaskLow);
  }

  // Nothing selected, behaves as if nothing is pressed
  // (HIGH bit indicates no button press)
  return JOIP | bitmaskLow;
}

word AddressBus::read(const dword address) {
  if (address < 0x100u && isBootRomEnabled()) {
    return BOOT_ROM[address];
  }

  // Joypad status register
  if (address == 0xFF00) {
    return getJoypad();
  }

  if (!refersToCartridge(address)) {
   if (address == 0xff07) {
     // FIXME there is a fucking sigsegv when reading this idk why
     //EXC_BAD_ACCESS (code=1, address=0xff18)
     // They suggest it is something related to threading...
   }
   if ((long long)this < 10) {
     // Interrupt here before sigsegv
   }
    return memory[address];
  }

  if (!isCartridgeInserted()) {
    return 0xFF;
  }

  return cart->read(address);
}

void AddressBus::write(const dword address, const word value, Component whois) {
  // Gameboy is allowed to do "forced" writes. This is used to skip bootrom, for
  // example, or to set "hardware" registers.
  if (whois == GB) {
    assert(!refersToCartridge(address));
    memory[address] = value;
  }

  if (refersToCartridge(address)) {
    assert(isCartridgeInserted() && "Trying to write to Cartridge without any inserted. This should not be possible, as boot rom does not perform write operations.");
    cart->write(address, value);
    return;
  }

  // Joypad status register
  // if (address == 0xFF00) {}
  // Nothing to do here. In this register, the lower nibble is read-only. However, in this
  // implementation all the button select logic is done in the read.

  if (address == 0xFF41) {
    // The three lower bits are only writable by PPU!
    const word mask = (whois == Ppu ? 0b00000111 : 0b11111000);
    memory[address] &= ~mask;
    memory[address] |= value & mask;
    return;
  }

  if (address == 0xFF44 && whois != Ppu) {
    return;
  }

  memory[address] = value;

  // Serial communication
  // todo add some proper interface
  if (address == 0xFF02 && value == 0x81) {
    gameboy->serialBuffer += read(0xFF01);
    return;
  }


  // TOdo DMA transfer register
  if (address == 0xFF46) {
    // Only allowed values are between 00 and DF
    if (value > 0xDF) {
      return;
    }
    // The transfer takes 160 M-cycles:
    // 640 dots (1.4 lines) in normal speed,
    // or 320 dots (0.7 lines) in CGB Double Speed Mode.
    //This is much faster than a CPU-driven copy.
    // Yeah we are not gonna care about that. Look at me, doing unsafe memory operations:
    //memcpy(&memory[0] + 0xFE00, &cart->getRom() + value * 0x100, 0xA0);
    // ^^ doesn't work
    for (int i = 0; i !=  0xA0; ++i) {
      memory[0xFE00 + i] = read(value * 0x100 + i);
    }

  }

  // Some edge cases in the book:
  if (address >= 0xE000 && address <= 0xFE00) {
    memory[address - 0x2000] = value;
    memory[address] = value;
    return;
  }

  if (address >= 0xC000 && address <= 0xDE00) {
    memory[address + 0x2000] = value;
    memory[address] = value;
    return;
  }
}


}