#include <stdexcept>
#include "cartridge.hpp"
#include "types.hpp"
#include "cartridge-types.hpp"

namespace gb {

// Header stuff/////////////////////////////////////////////////////////////////
// This parses the header without checking if it's valid.
// Original DMG did not check validity and so the emulator should neither.
Cartridge::Header::Header(const std::vector<word>& rom)  {
  if (rom.size() < MIN_ROM_BANKS * ROM_BANK_SIZE) {
    throw std::runtime_error("Tried to parse an empty or invalid ROM header.");
  }

  std::copy(rom.begin() + CARTHEADER_LOWER_BOUND , rom.begin() + CARTHEADER_UPPER_BOUND, title.begin());
  license[0]        = rom[CARTHEADER_LICENSE_0];
  license[1]        = rom[CARTHEADER_LICENSE_1];
  sgbFlag           = rom[CARTHEADER_SGBFLAG];
  cartridgeType     = rom[CARTHEADER_TYPE];
  ROMSize           = rom[CARTHEADER_ROMSIZE];
  RAMSize           = rom[CARTHEADER_RAMSIZE];
  destinationCode   = rom[CARTHEADER_DESTINATION];
  oldLicense        = rom[CARTHEADER_OLDLICENSE];
  versionNumber     = rom[CARTHEADER_VERSION];
  headerChecksum    = rom[CARTHEADER_CHECKSUM];
  globalChecksum[0] = rom[CARTHEADER_GLOBALCHECKSUM_0];
  globalChecksum[1] = rom[CARTHEADER_GLOBALCHECKSUM_1];

  // Check if cartridge should be battery backed.
  switch (cartridgeType) {
    case MBC3_TIMER_RAM_BATTERY:
    case MBC3_TIMER_BATTERY:
    case MBC3_RAM_BATTERY:
    case MBC1_RAM_BATTERY:
    case MBC2_BATTERY:
    case MBC5_RAM_BATTERY:
    case MBC5_RUMBLE_RAM_BATTERY:
    case MBC7_SENSOR_RUMBLE_RAM_BATTERY:
    case MMM01_RAM_BATTERY:
    case ROM_RAM_BATTERY:
      isBatteryBacked = true;
      break;

    default:
      isBatteryBacked = false;
      break;
  }
}
////////////////////////////////////////////////////////////////////////////////

// Static Methods //////////////////////////////////////////////////////////////
// This is a static method. It looks like that it can be replaced by
// the parsing done in Header, but this actually needs to exist. It is used
// to check which MBC type should be istantiated (and so it gets called
// before header parsing can take place).
Cartridge::MBCType Cartridge::getMBC(const Binary& rom) {
  if (rom.size() < MIN_ROM_BANKS * ROM_BANK_SIZE) {
    throw std::runtime_error("Tried to parse an empty or invalid ROM.");
  }

  return static_cast<MBCType>(rom[CARTHEADER_TYPE]);
}

// Constructor /////////////////////////////////////////////////////////////////
// Instantiate cartridge from binary data
Cartridge::Cartridge(const Binary& data) : header{data} {
  // Check that ROM is valid by having an integer number of banks
  if (data.size() % ROM_BANK_SIZE != 0) {
    throw std::runtime_error("Invalid ROM file: blocks must be multiples of 16KiB!");
  }

  // Store ROM data in memory
  // Binary is a vector of words; using it with call syntax instantiates a vector
  // of n elements se to zero.
  rom = Binary(data.size());
  std::copy(data.begin(), data.end(), rom.begin());
  assert(rom.size() == data.size());

  setRAMSize();
}

// Methods /////////////////////////////////////////////////////////////////////
const Cartridge::Header& Cartridge::getHeader() const {
  return header;
}

const Binary& Cartridge::getRom() {
  return rom;
}

void Cartridge::setRAMSize() {
  switch (header.RAMSize) {
    case 1:
      ram = std::vector<word>(0x800);
      break;

    case 2:
      ram = std::vector<word>(0x2000);
      break;

    case 3:
      ram = std::vector<word>(0x8000);
      break;

    default:
      ram = std::vector<word>(0);
      break;
  }
}

void Cartridge::loadBatteryBackedRAM(Binary newRam) {
  if (newRam.size() != ram.size()) {
    throw std::runtime_error("Trying to load a save game of invalid size for this cartridge!");
  }

  std::copy(newRam.begin(), newRam.end(), ram.begin());
}

const Binary& Cartridge::getRam() {
  return ram;
}

} // namespace gb