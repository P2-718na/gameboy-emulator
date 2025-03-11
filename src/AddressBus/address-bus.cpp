#include "address-bus.hpp"
#include "types.hpp"
#include "cartridge.hpp"

#include <cassert>
#include <gameboy.hpp>

namespace gb {

constexpr std::array<word, BOOTROM_UPPER_BOUND> AddressBus::BOOT_ROM;

// Constructor /////////////////////////////////////////////////////////////////
AddressBus::AddressBus(Gameboy* gameboy) : gameboy{gameboy} {
  // Leaving these asserts here because I have no better place to put them.
  assert(sizeof(word) == 1);
  assert(sizeof(dword) == 2);
}

// Methods /////////////////////////////////////////////////////////////////////
bool AddressBus::isBootRomEnabled() const {
  // 1 = disabled, 0 = enabled
  return !(memory[BOOT_ROM_LOCK] & 1);
}

bool AddressBus::isCartridgeInserted() const {
  return cart != nullptr;
}

bool AddressBus::refersToCartridge(gb::dword address) {
  return (address < CART_ROM_UPPER_BOUND) || (address >= CART_RAM_LOWER_BOUND && address < CART_RAM_UPPER_BOUND);
}

void AddressBus::loadCart(Cartridge* newCart) {
  cart = newCart;
};

word AddressBus::getJoypad() const {
  const word joypadStatus = gameboy->joypadStatus;
  const word JOIP = memory[REG_JOIP];
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

// Todo this should be refactored to be clearer.
word AddressBus::read(const dword address) const {
  if (address < BOOTROM_UPPER_BOUND && isBootRomEnabled()) {
    return BOOT_ROM[address];
  }

  // Joypad status register
  if (address == REG_JOIP) {
    return getJoypad();
  }

  // TAC Register.
  // From docs, only the lowest three bits of this register matter.
  // It does not say if the output should be masked or not.
  // I am keeping this here just for reference.
  //if (address == 0xFF07) {
  //  return memory[address] & 0b111;
  //}

  if (!refersToCartridge(address)) {
    return memory[address];
  }

  if (!isCartridgeInserted()) {
    return 0xFF;
  }

  return cart->read(address);
}

// Todo this should be refactored, same as read
void AddressBus::write(const dword address, const word value, Component whois) {
  // Gameboy is allowed to do "forced" writes. This is used to skip bootrom, for
  // example, or to set "hardware" registers.
  if (whois == GB) {
    assert(!refersToCartridge(address));
    memory[address] = value;
    return;
  }

  if (refersToCartridge(address)) {
    assert(isCartridgeInserted() &&
           "Trying to write to Cartridge without any inserted."
           "This should not be possible, as boot rom does not perform write operations.");
    cart->write(address, value);
    return;
  }

  // Joypad status register: no need to do anyhting special
  // In my implementation all the button select logic is done in the read.

  if (address == REG_STAT) {
    // The three lower bits are only writable by PPU!
    const word mask = (whois == PPU ? 0b00000111 : 0b11111000);
    memory[address] &= ~mask;
    memory[address] |= value & mask;
    return;
  }

  if (address == REG_LY && whois != PPU) {
    return;
  }

  memory[address] = value;

  // Placeholder for serial communication.
  // Todo implement proper serial stuff
  if (address == REG_SC && value == 0x81) {
    gameboy->serialBuffer += static_cast<char>(read(REG_SB));
    return;
  }

  if (address == REG_DMA) {
    // Only allowed values are between 00 and E0 (not included)
    if (value >= 0xE0) {
      return;
    }
    // The transfer takes 160 M-cycles:
    // 640 dots (1.4 lines) in normal speed,
    // or 320 dots (0.7 lines) in CGB Double Speed Mode.
    // Yeah we are not gonna care about that. Look at me, doing unsafe memory operations:
    // memcpy(&memory[0] + 0xFE00, &cart->getRom() + value * 0x100, 0xA0);
    // ^^ doesn't work. Leaving it here for the future.
    // 0xA0 = 160, number of addresses to copy
    for (int i = 0; i !=  0xA0; ++i) {
      memory[OAM_MEMORY_LOWER_BOUND + i] = read(value * 0x100 + i);
    }
  }

  // Some edge cases in the book:
  if (address >= ECHO_RAM_LOWER_BOUND_0 && address < ECHO_RAM_UPPER_BOUND_0) {
    memory[address - 0x2000] = value;
    memory[address] = value;
    return;
  }

  if (address >= ECHO_RAM_LOWER_BOUND_1 && address < ECHO_RAM_UPPER_BOUND_1) {
    memory[address + 0x2000] = value;
    memory[address] = value;
    return;
  }

  // Todo add FEA0–FEFF range edge case, see pandocs
}


}