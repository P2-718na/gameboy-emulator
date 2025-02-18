#include <array>
#include "types.hpp"

#ifndef CARTRIDGE_H
#define CARTRIDGE_H

namespace gb {

class Cartridge {
 public:
  typedef std::vector<word> Rom;

  // Size in bytes of a single rom bank
  static constexpr unsigned int ROM_BANK_SIZE = 0x4000u;

  struct Header {
    std::array<char, 16> title{};
    std::array<char, 2> license{};
    word sgbFlag;
    word cartridgeType;
    word ROMSize;
    word RAMSize;
    word destinationCode;
    word oldLicense;
    word versionNumber;
    word headerChecksum;
    std::array<char, 2> globalChecksum{};
    bool isBatteryBacked;

    Header() = delete;
    explicit Header(const Rom& rom);
  };

  typedef enum : word {
    MBC0 = 0x00,
    MBC1 = 0x01,
    MBC1_RAM = 0x02,
    MBC1_RAM_BATTERY = 0x03,
    MBC2 = 0x05,
    MBC2_BATTERY = 0x06,
    ROM_RAM = 0x08,
    ROM_RAM_BATTERY = 0x09,
    MMM01 = 0x0B,
    MMM01_RAM = 0x0C,
    MMM01_RAM_BATTERY = 0x0D,
    MBC3_TIMER_BATTERY = 0x0F,
    MBC3_TIMER_RAM_BATTERY = 0x10,
    MBC3 = 0x11,
    MBC3_RAM = 0x12,
    MBC3_RAM_BATTERY = 0x13,
    MBC5 = 0x19,
    MBC5_RAM = 0x1A,
    MBC5_RAM_BATTERY = 0x1B,
    MBC5_RUMBLE = 0x1C,
    MBC5_RUMBLE_RAM = 0x1D,
    MBC5_RUMBLE_RAM_BATTERY = 0x1E,
    MBC6 = 0x20,
    MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
    POCKET_CAMERA = 0xFC,
    BANDAI_TAMA5 = 0xFD,
    HuC3 = 0xFE,
    HuC1_RAM_BATTERY = 0xFF
  } MBCType;

  static MBCType getMBC(const Rom& rom);

  // Constructors /////////////////////////////////////////////
  Cartridge() = delete;
  explicit Cartridge(const Rom& rom);
  // This prevents some possible undefined behavior when using smart pointers and inheritance.
  // (This happens when we call the destructor of the base class on an element that is of a derived class).
  virtual ~Cartridge() = default;

  // These methods get called by addressBus whenever a read/write occurs in ROM region.
  const Rom& getRom();
  virtual word read(dword address) = 0;
  virtual void write(dword address, word value) = 0;

  const Header& getHeader() const;
  void loadExternalRam(Rom newRam);
  const Rom& getRam();

 private:
  Header header;

 protected:
  Rom rom;
  std::vector<word> ram;

  void initRAM();
};

}

#endif  // CARTRIDGE_H
