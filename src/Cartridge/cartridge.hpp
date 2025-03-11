#include <array>
#include "types.hpp"

#ifndef CARTRIDGE_H
#define CARTRIDGE_H

namespace gb {


// Todo this code should be refactored.
//  Cartridge should just be a normal class,
//  and I should add the abstract class "Controller", with all its derivatives MBC0, MBC1...
//  Since I am supporting only a few cartridges, this is fine for now.

// To implement new cartridge types, one must create the respective hpp file in
// the Controllers folder. Then, include it in cartridge-types.hpp (so that
// it gets compiled properly). Then, implement all the pure virtual methods
// and it's done.
// Expect to find a few hardcoded values in the various mbc implementations
// (mostly for special addresses). These are all taken stright out of
// the documentation and I believe they make it easier to read and write
// mbc code (rather than having them all be separate variables).
class Cartridge {
 public:
  // Size in bytes of a single rom bank
  static constexpr unsigned int ROM_BANK_SIZE{0x4000u};

  // Quick and dirty header parser.
  struct Header {
    std::array<word, 16> title{};
    std::array<word, 2> license{};
    word sgbFlag;
    word cartridgeType;
    word ROMSize;
    word RAMSize;
    word destinationCode;
    word oldLicense;
    word versionNumber;
    word headerChecksum;
    std::array<word, 2> globalChecksum{};
    bool isBatteryBacked;

    Header() = delete;
    explicit Header(const Binary& rom);
  };

  // MCB Type is defined by a number in cartridge header. These are all known types.
  // Some multirom cartridges can not be detected by only looking at this value.
  // They are not supported here.
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

  static MBCType getMBC(const Binary& rom);

  // Constructors //////////////////////////////////////////////////////////////
  Cartridge() = delete;
  explicit Cartridge(const Binary& rom);

  // This prevents some possible undefined behavior when using smart pointers and inheritance.
  // (This happens when we call the destructor of the base class on an element that is of a derived class).
  virtual ~Cartridge() = default;

  // And since we are manually defining a destructor, we need to respect the
  // rule of five and also manually define the others...
  Cartridge(const Cartridge& copyFrom) = default;
  Cartridge& operator=(const Cartridge& copyFrom) = default;
  Cartridge(Cartridge &&) = default;
  Cartridge& operator=(Cartridge &&) = default;
  //////////////////////////////////////////////////////////////////////////////

  // These methods get called by addressBus whenever a read/write occurs in ROM region.
  const Binary& getRom();
  virtual word read(dword address) = 0;
  virtual void write(dword address, word value) = 0;

  const Header& getHeader() const;
  void loadBatteryBackedRAM(Binary newRam);
  const Binary& getRam();

 private:
  Header header;

 protected:
  Binary rom;
  std::vector<word> ram;

  void setRAMSize();
};

}

#endif  // CARTRIDGE_H
