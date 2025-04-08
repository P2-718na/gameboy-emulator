#include "cartridge.hpp"
#include <vector>
#include <string>
#include "doctest.h"

using namespace gb;

// Helper function to create a minimal valid ROM binary
Binary createTestROM(Cartridge::MBCType mbcType = Cartridge::MBCType::MBC0) {
  std::vector<word> rom(0x8000, 0);  // 32KB minimum ROM size

  // Set cartridge type
  rom[0x147] = static_cast<word>(mbcType);

  // Set ROM size (32KB = 0x00)
  rom[0x148] = 0x00;

  // Set RAM size (no RAM = 0x00)
  rom[0x149] = 0x00;

  // Set a simple title
  const std::string title = "TEST ROM";
  std::copy(title.begin(), title.end(), rom.begin() + 0x134);

  return { rom.begin(), rom.end() };
}

TEST_CASE("Cartridge Header Parsing") {
  Binary rom = createTestROM();
  Cartridge::Header header(rom);

  SUBCASE("Basic Header Fields") {
    // Check title
    std::string title;
    for (int i = 0; i < 16 && header.title[i] != 0; i++) {
      title += static_cast<char>(header.title[i]);
    }
    CHECK_EQ(title, "TEST ROM");

    // Check cartridge type
    CHECK_EQ(header.cartridgeType, Cartridge::MBCType::MBC0);

    // Check ROM size
    CHECK_EQ(header.ROMSize, 0x00);  // 32KB

    // Check RAM size
    CHECK_EQ(header.RAMSize, 0x00);  // No RAM

    // Check battery-backed status
    CHECK_FALSE(header.isBatteryBacked);
  }
}

TEST_CASE("Cartridge MBC Type Detection") {
  SUBCASE("MBC0 Detection") {
    Binary rom = createTestROM(Cartridge::MBCType::MBC0);
    CHECK_EQ(Cartridge::getMBC(rom), Cartridge::MBCType::MBC0);
  }

  SUBCASE("MBC1 Detection") {
    Binary rom = createTestROM(Cartridge::MBCType::MBC1);
    CHECK_EQ(Cartridge::getMBC(rom), Cartridge::MBCType::MBC1);
  }

  SUBCASE("MBC1 with RAM Detection") {
    Binary rom = createTestROM(Cartridge::MBCType::MBC1_RAM);
    CHECK_EQ(Cartridge::getMBC(rom), Cartridge::MBCType::MBC1_RAM);
  }

  SUBCASE("MBC1 with RAM and Battery Detection") {
    Binary rom = createTestROM(Cartridge::MBCType::MBC1_RAM_BATTERY);
    CHECK_EQ(Cartridge::getMBC(rom), Cartridge::MBCType::MBC1_RAM_BATTERY);
  }
}


TEST_CASE("Cartridge Edge Cases") {
  SUBCASE("Invalid ROM Size") {
    // Create a ROM that's too small
    std::vector<word> smallRom(0x100, 0);

    // Rom shound be at least two banks big
    CHECK_THROWS(Cartridge::getMBC(smallRom));


    // This should throw an exception or handle the error gracefully
    CHECK_THROWS(Cartridge::Header(smallRom));
  }
}