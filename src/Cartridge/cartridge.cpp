#include <stdexcept>
#include "cartridge.hpp"
#include "cartridge-types.hpp"
#include "gameboy.hpp"

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

Cartridge::MBCType Cartridge::getMBC(const Binary& rom) {
  return static_cast<MBCType>(rom[0x147]);
}

Cartridge::Cartridge(const Binary& data) : header{data} {
  // Check that ROM is valid by having a finite number of banks
  if (data.size() % ROM_BANK_SIZE != 0) {
    throw std::runtime_error("Invalid ROM file: blocks must be multiples of 16KiB!");
  }

  // Store ROM data in memory
  rom = Binary(data.size());
  std::copy(data.begin(), data.end(), rom.begin());
  assert(rom.size() == data.size());

  initRAM();
}

const Cartridge::Header& Cartridge::getHeader() const {
  return header;
}

const Binary& Cartridge::getRom() {
  return rom;
}

void Cartridge::initRAM() {
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

void Cartridge::loadExternalRam(Binary newRam) {
  if (newRam.size() != ram.size()) {
    throw std::runtime_error("Trying to load a save game of invalid size for this cartridge!");
  }

  std::copy(newRam.begin(), newRam.end(), ram.begin());
}

const Binary& Cartridge::getRam() {
  return ram;
}
}