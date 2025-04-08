#include "gameboy.hpp"
#include <vector>
#include "doctest.h"
#include "types.hpp"

using namespace gb;

// Helper function to create a minimal test ROM
Binary createMinimalTestROM() {
  std::vector<word> rom(0x8000, 0);  // 32KB minimum ROM size

  // Set cartridge type (MBC0 - ROM ONLY)
  rom[0x147] = 0x00;

  // Set ROM size (32KB)
  rom[0x148] = 0x00;

  // Set RAM size (None)
  rom[0x149] = 0x00;

  return { rom.begin(), rom.end() };
}

TEST_CASE("Gameboy Initialization") {
  Binary rom = createMinimalTestROM();
  Gameboy gameboy(rom);

  SUBCASE("Initial State") {
    // Screen should not be on by default
    CHECK_FALSE(gameboy.isScreenOn());

    // Serial buffer should be empty
    CHECK(gameboy.serialBuffer.empty());

    // Screen buffer should be initialized to all zeros (black)
    for (const auto& pixel : gameboy.screenBuffer) {
      CHECK_EQ(pixel.to_ulong(), 0);
    }

    // Should not have battery-backed save by default with ROM ONLY cartridge
    CHECK_FALSE(gameboy.shouldSave());
  }

  SUBCASE("Boot sequence") {
    for (int i = 0; i != 100000; ++i) {
      gameboy.machineClock();
    }

    // Screen should be turned on by boot rom after a few cycles.
    CHECK(gameboy.isScreenOn());
  }
}

TEST_CASE("Gameboy Save State") {
  SUBCASE("ROM Only (No Save)") {
    Binary rom = createMinimalTestROM();
    Gameboy gameboy(rom);

    CHECK_FALSE(gameboy.shouldSave());

    // Attempting to load save for not battery-backed data should cause issues.
    std::vector<word> save_data(0x2000, 0xFF);
    Binary save(save_data.begin(), save_data.end());
    CHECK_THROWS(gameboy.loadSave(save));
  }

  SUBCASE("Load a save of wrong size") {
    Binary rom = createMinimalTestROM();
    Gameboy gameboy(rom);

    CHECK_FALSE(gameboy.shouldSave());

    // Attempting to load save for not battery-backed data should cause issues.
    std::vector<word> save_data(0x111, 0xFF);
    Binary save(save_data.begin(), save_data.end());
    CHECK_THROWS(gameboy.loadSave(save));
  }
}

TEST_CASE("Gameboy Serial Communication") {
  Binary rom = createMinimalTestROM();
  Gameboy gameboy(rom);

  SUBCASE("Serial Buffer Operations") {
    // Initially empty
    CHECK(gameboy.serialBuffer.empty());

    // Printing should clear buffer.
    gameboy.serialBuffer = "Testing serial buffer...";
    gameboy.printSerialBuffer();
    CHECK(gameboy.serialBuffer.empty());
  }
}

TEST_CASE("Loading ROM of unsupported MBC Type") {
  // Create a ROM with an invalid MBC type
  Binary rom = createMinimalTestROM();  // Invalid/unsupported type
  // Set MBC type as unsupported
  rom[0x147] = 0x42;

  // Gameboy has to throw.
  CHECK_THROWS(Gameboy{ rom });
}