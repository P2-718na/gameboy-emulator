#ifndef CARTRIDGE_MBC3
#define CARTRIDGE_MBC3

#include "RTC/rtc.hpp"

namespace gb {

class MBC3 : public Cartridge {

  unsigned int romBank{};
  unsigned int ramBank{};
  RTC timer{};
  bool readyForLatch{false};
  bool externalRamEnabled{false};

 public:
  // TODO
  explicit inline MBC3(const Rom& rom) : Cartridge{rom} {};

  inline word read(const dword address) override {
    if (address < 0x4000u) {
      return rom[address];
    }

    if (address < 0x8000u) {
      return rom[0x4000 * romBank + (address - 0x4000)];
    }

    assert(address >= 0xA000 && "Cartridge controller was asked to write outside of its memory!");
    assert(address < 0xC000 && "Cartridge controller was asked to write outside of its memory!");

    // IF These addresses are mapped to RAM...
    if (ramBank < 0x04 && externalRamEnabled) {
      const dword ramAddress = 0x2000 * ramBank + (address - 0xA000);
      return ram[ramAddress];
    }

    // IF instead they are mapped to RTC...
    if (0x08 <= ramBank && ramBank < 0x0D) {
      return timer.getLatchedRegister(ramBank);
    }

    // If external RAM is not enabled
    // (Or if we are in a different case: ROM's fault)
    return 0xFF;

  }


  // Todo add battery-backed writes
  inline void write(const dword address, const word value) override {
    if (address < 0x2000u) {
      externalRamEnabled = (value & 0b1111) == 0xA;
      return;
    }

    if (address < 0x4000u) {
      const int bank = value & 0b1111111;
      romBank = bank != 0 ? bank : 1;
      return;
    }

    if (address < 0x6000u) {
      ramBank = value;
      return;
    }

    if (address < 0x8000u) {
      // I really don't know if the two writes for the latch have to be consecutive.
      if (value == 0x00) {
        readyForLatch = true;
        return;
      }

      if (value == 0x01 && readyForLatch) {
        timer.latch();
        readyForLatch = false;
        return;
      }
    }

    assert(address >= 0xA000 && "Cartridge controller was asked to write outside of its memory!");
    assert(address < 0xC000 && "Cartridge controller was asked to write outside of its memory!");

    if (externalRamEnabled && ramBank < 0x04) {
      const dword ramAddress = 0x2000 * ramBank + (address - 0xA000);
      ram[ramAddress] = value;
      return;
    }

    if (0x08 <= ramBank && ramBank < 0x0D) {
      timer.writeRegister(ramBank, value);
      return;
    }

    // Otherwise, ignore the write.
  }
};

}

#endif