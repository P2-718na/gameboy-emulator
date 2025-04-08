#include "doctest.h"
#include "address-bus.hpp"
#include "gameboy.hpp"
#include "cartridge.hpp"
#include "cartridge-types.hpp"
#include "types.hpp"
#include <memory>
#include <cstdlib>

using namespace gb;
// FIXME fix indentation in these tests

TEST_CASE("AddressBus Basic Operations") {
    Gameboy gameboy{std::vector<word>(0x8000, 0)};  // Create with empty ROM
    AddressBus bus{&gameboy};

    SUBCASE("Initial State") {
        CHECK_FALSE(bus.isCartridgeInserted());
        CHECK(bus.isBootRomEnabled());  // Boot ROM should be enabled initially

        // Writing anything to cartridge RAM without cartridge inserted should throw.
        for (dword testAddr = 0xA000; testAddr != 0xC000; ++testAddr) {
          CHECK(AddressBus::refersToCartridge(testAddr));
          CHECK_THROWS(bus.write(testAddr, 0x00));
        }
    }

    SUBCASE("Boot ROM Access") {
        // When boot ROM is enabled, reading from 0x0000-0x00FF should return boot ROM data
        CHECK(bus.isBootRomEnabled());
        // Test first few bytes of boot ROM (known values)
        CHECK_EQ(bus.read(0x0000), 0x31);  // First instruction of boot ROM
        CHECK_EQ(bus.read(0x0001), 0xFE);
    }

    SUBCASE("Disabling BOOT ROM") {
        CHECK(bus.isBootRomEnabled());  // Boot ROM should be enabled initially
        // Setting this to 1 disables BOOT ROM
        bus.write(BOOT_ROM_LOCK, 0x01);
        CHECK_FALSE(bus.isBootRomEnabled());
        // Check if it actually switched off boot rom
        CHECK_NE(bus.read(0x0000), 0x31);  // First instruction of boot ROM
        CHECK_NE(bus.read(0x0001), 0xFE);
    }

    SUBCASE("Memory Read/Write Basic") {
        // Seed for reproducibility
        std::srand(0);

        // Test writing and reading from internal RAM area (0xC000-0xDFFF)
        // In this address range, data is written and read as-is.
        // Writing/reading here should be possible even without cartridge inserted.
        for (dword testAddr = 0xC000; testAddr != 0xE000; ++testAddr) {
          CHECK_FALSE(AddressBus::refersToCartridge(testAddr));

          // rand does the job just fine.
          const word testValue = std::rand();
          bus.write(testAddr, testValue);

          CHECK_EQ(bus.read(testAddr), testValue);
        }
    }


    SUBCASE("Writing to PPU special addresses") {
        // The three lower bits are only writable by PPU!
        bus.write(REG_STAT, 0b111, AddressBus::PPU);
        CHECK_EQ(bus.read(REG_STAT), 0b111);
        bus.write(REG_STAT, 0b000, AddressBus::CPU);
        CHECK_EQ(bus.read(REG_STAT), 0b111);

        // REG_LY can only be written by PPU
        bus.write(REG_LY, 0x42, AddressBus::PPU);
        CHECK_EQ(bus.read(REG_LY), 0x42);
        bus.write(REG_LY, 0x00, AddressBus::CPU);
        CHECK_EQ(bus.read(REG_LY), 0x42);
    }
}

TEST_CASE("AddressBus Echo RAM") {
    Gameboy gameboy{std::vector<word>(0x8000, 0)};
    AddressBus bus{&gameboy};


  SUBCASE("Echo RAM Behavior") {
      // Seed for reproducibility
      std::srand(0);

      // Writing to work RAM should be reflected in echo RAM
      // Writing to echo RAM should be reflected in work RAM
      for (dword testAddr = 0xC000; testAddr != 0xDE00; ++testAddr) {
          const dword echoAddr = testAddr + 0x2000;
          const word testValue1 = std::rand();
          // Ensure the test values are different
          const word testValue2 = testValue1 + 1;

          bus.write(testAddr, testValue1);
          CHECK_EQ(bus.read(echoAddr), testValue1);

          bus.write(echoAddr, testValue2);
          CHECK_EQ(bus.read(testAddr), testValue2);
      }
    }
}



TEST_CASE("AddressBus Cartridge Operations") {
    auto rom = std::vector<word>(0x8000, 0);

    std::srand(0);
    // Fill ROM with random values (after the header)
    for (dword addr = 0x200; addr != 0x8000; ++addr) {
      rom[addr] = std::rand();
    }

    Gameboy gameboy{rom};
    AddressBus bus{&gameboy};
    // Load empty ROM to address bus
    const auto cart = std::make_unique<MBC0>(rom);
    bus.loadCart(cart.get());

    SUBCASE("Cartridge Reference Check") {
      // Test ROM bank 0 (0x0000-0x3FFF)
      for (dword testAddr = 0x0000; testAddr != 0x8000; ++testAddr) {
        CHECK(AddressBus::refersToCartridge(testAddr));
      }

      for (dword testAddr = 0xA000; testAddr != 0xC000; ++testAddr) {
         CHECK(AddressBus::refersToCartridge(testAddr));
      }

      for (dword testAddr = 0x8000; testAddr != 0xA000; ++testAddr) {
         CHECK_FALSE(AddressBus::refersToCartridge(testAddr));
      }

      // All other addresses are internal addresses
      for (dword testAddr = 0xC000; testAddr != 0x0000; ++testAddr) {
         CHECK_FALSE(AddressBus::refersToCartridge(testAddr));
      }
    }

    SUBCASE("Memory Regions Write Protection") {
      // Seed for reproducibility.
      // This should be different than the seed used to initialize rom.
      std::srand(1);

      // Try to write to ROM area (should be ignored)
      for (dword testAddr = 0x0000; testAddr != 0x8000; ++testAddr) {
         const word originalValue = bus.read(testAddr);
         bus.write(testAddr, std::rand());
         CHECK_EQ(bus.read(testAddr), originalValue);  // Value should not change
      }
    }
}
