#ifndef CARTRIDGE_MBC0
#define CARTRIDGE_MBC0

#include <cassert>

namespace gb {

class MBC0 : public Cartridge {

 public:
  explicit inline MBC0(const Binary& rom) : Cartridge{rom} {};

  inline word read(const dword address) override {
    // This case would indicate errors in my code.
    // Fixme addresses
    assert((address < 0x8000 || (0xC000 > address && address >= 0xA000) ) && "Cartridge controller was asked to write outside of its memory!");
    // ROM-Only cartridges have fixed size.
    assert(rom.size() == 2 * ROM_BANK_SIZE);

    return rom[address];
  }

  inline void write(const dword address, const word value) override {
    // This case would indicate errors in my code.
    // Fixme addresses
    assert((address < 0x8000 || (0xC000 > address && address >= 0xA000) ) && "Cartridge controller was asked to write outside of its memory!");

    // Otherwise, writes to ROM are ignored.
  }
};

}

#endif