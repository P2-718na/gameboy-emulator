#ifndef CARTRIDGE_MBC1
#define CARTRIDGE_MBC1

#include <vector>
#include <stdexcept>

namespace gb {

class MBC1 : public Cartridge {
  unsigned int romBank{1};
  unsigned int ramBank{0};
  bool modeFlag{false};
  bool externalRamEnabled{false};

 public:
  explicit inline MBC1(const Binary& rom) : Cartridge{rom} {};

  inline word getBankBitmask() const {
    const auto header = getHeader();
    switch (header.ROMSize) {
      case 0:
        return 0b1;
      case 1:
        return 0b11;
      case 2:
        return 0b111;
      case 3:
        return 0b1111;
      case 4:
      case 5:
      case 6:
        return 0b11111;

      default:
        throw std::runtime_error("Rom size is invalid or unsupported. Check your ROM file.");
    }
  }

  inline dword getRamAddress(dword address) const {
    const auto ramSize = ram.size();
    // Code tried to access RAM even though it is disabled.
    // This COULD happen because of ROM code's fault, but it must be catched
    // (and ignored) before it gets here.
    assert(ramSize != 0 && "Cartridge has no ram but it tried writing to it.");

    const dword baseAddress = address - 0xA000;
    if (modeFlag) {
      return 0x2000 * ramBank + baseAddress;
    }

    return baseAddress % ramSize;
  }

  inline word getZeroBank() const {
    // If mode flag 0, result is always zero.
    if (!modeFlag) {
      return 0;
    }

    const auto header = getHeader();
    // If ROM size is less than 1MB, number is always zero.
    if (header.ROMSize < 0x05) {
      return 0;
    }

    // Exactly 1MB ROMs
    if (header.ROMSize == 0x05) {
      const word zeroBank = (ramBank & 0b1) << 5;
      assert((zeroBank == 0x00 || zeroBank == 0x20) && "zeroBank number outside of possible values.");
      return zeroBank;
      // NOTE: This does not work for "MBC1M" carts. These have some differences which can
      // essentially not be detected in software. These will not be supported for now.
    }

    // Exactly 2MB ROMs
    if (header.ROMSize == 0x06) {
      const word zeroBank = (ramBank & 0b11) << 5;
      assert((zeroBank == 0x00 || zeroBank == 0x20 || zeroBank == 0x40 || zeroBank == 0x60) && "zeroBank number outside of possible values.");
      return zeroBank;
    }

    // MBC1 Roms have MAX 2MB. If code jumps here, it means that the header is wrong.
    // We ignore this case.
    return 0;
  }

  inline unsigned int getHighBank() const {
    const auto header = getHeader();
    const word bitmask = getBankBitmask();

    // ROM size is less than 1MB.
    if (header.ROMSize < 0x05) {
      return romBank & bitmask;
    }

    // Exactly 1MB ROMs
    if (header.ROMSize == 0x05) {
      return romBank & bitmask & ((ramBank & 0b1) << 5);
    }

    // Exactly 2MB ROMs
    if (header.ROMSize == 0x06) {
      return romBank & bitmask & ((ramBank & 0b11) << 5);
    }

    // MBC1 Roms have MAX 2MB. If code jumps here, it means that the header is wrong.
    // We ignore this case.
    return 0;
  };

  inline word read(const dword address) override {
    if (address < 0x4000u) {
      return rom[0x4000 * getZeroBank() + address];
    }

    if (address < 0x8000u) {
      return rom[0x4000 * getHighBank() + (address - 0x4000)];
    }

    assert(address >= 0xA000 && "Cartridge controller was asked to write outside of its memory!");
    assert(address < 0xC000 && "Cartridge controller was asked to write outside of its memory!");

    if (externalRamEnabled && !ram.empty()) {
      const dword ramAddress = getRamAddress(address);
      return ram[ramAddress];
    }

    // Invalid read, returns 0xFF.
    return 0xFF;
  }

  // Todo add battery-backed writes
  inline void write(const dword address, const word value) override {
    if (address < 0x2000u) {
      externalRamEnabled = (value & 0b1111) == 0xA;
      return;
    }

    if (address < 0x4000u) {
      const word bitmask = getBankBitmask();
      const int bank = value & bitmask;
      romBank = bank != 0 ? bank : 1;
      return;
    }

    if (address < 0x6000u) {
      ramBank = value & 0b11;
      return;
    }

    if (address < 0x8000u) {
      modeFlag = value & 0b1;
    }

    assert(address >= 0xA000 && "Cartridge controller was asked to write outside of its memory!");
    assert(address < 0xC000 && "Cartridge controller was asked to write outside of its memory!");

    if (externalRamEnabled && !ram.empty()) {
      const dword ramAddress = getRamAddress(address);
      ram[ramAddress] = value;
    }

    // Otherwise ignore write
  }
};

}

#endif