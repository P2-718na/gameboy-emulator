#ifndef CARTRIDGE_MBC0
#define CARTRIDGE_MBC0

#include <cassert>

namespace gb {

class MBC0 : public Cartridge {

 public:
  explicit inline MBC0(const Binary& rom) : Cartridge{rom} {};

  inline word read(const dword address) override {
    // This case would indicate errors in my code.
    assert((address < CART_ROM_UPPER_BOUND || (CART_RAM_UPPER_BOUND > address && address >= CART_RAM_LOWER_BOUND) ) && "Cartridge controller was asked to read outside of its memory!");
    // ROM-Only cartridges have fixed size.
    assert(rom.size() == 2 * ROM_BANK_SIZE);

    return rom[address];
  }

  inline void write(const dword address, const word value) override {
    // This case would indicate errors in my code.
    assert((address < CART_ROM_UPPER_BOUND || (CART_RAM_UPPER_BOUND > address && address >= CART_RAM_LOWER_BOUND) ) && "Cartridge controller was asked to write outside of its memory!");

    // Otherwise, writes to ROM are ignored.
  }
};

}

#endif