#include "cartridge.hpp"
#include "cartridge-types.hpp"

namespace gb {

// Todo check that the header is valid (no conflicts between rom/ram size and MBC type
Cartridge::Header::Header(const std::vector<word>& rom)  {
  std::copy(rom.begin() + 0x134, rom.begin() + 0x143, title.begin());
  std::copy(rom.begin() + 0x144, rom.begin() + 0x145, license.begin());
  sgbFlag = rom[0x146];
  cartridgeType = rom[0x147];
  ROMSize = rom[0x148];
  RAMSize = rom[0x149];
  destinationCode = rom[0x14A];
  oldLicense = rom[0x14B];
  versionNumber = rom[0x14C];
  headerChecksum = rom[0x14D];
  std::copy(
    rom.begin() + 0x14E,
    rom.begin() + 0x14F,
    globalChecksum.begin());
}

Cartridge::MBCType Cartridge::getMBC(const Rom& rom) {
  return static_cast<MBCType>(rom[0x147]);
}

Cartridge::Cartridge(const Rom& data) : header{data} {
  // Check that ROM is valid by having a finite number of banks
  if (data.size() % ROM_BANK_SIZE != 0) {
    throw std::runtime_error("Invalid ROM file: blocks must be multiples of 16KiB!");
  }

  // Store ROM data in memory
  rom = Rom(data.size());
  std::copy(data.begin(), data.end(), rom.begin());
  assert(rom.size() == data.size());
}

const Cartridge::Header& Cartridge::getHeader() const {
  return header;
}

}