#include "timer-controller.hpp"
#include "address-bus.hpp"
#include "doctest.h"
#include "gameboy.hpp"
#include "types.hpp"

using namespace gb;

// These tests could be performed using test ROMs but, for example, Blargg's roms would require to have
// sub M-clock precision enabled for timers, which I do not have implemented here.
// These tests will do just fine.

TEST_CASE("TimerController Basic Operations") {
  Gameboy gameboy{ std::vector<word>(0x8000, 0) };  // Create with empty ROM
  AddressBus bus{ &gameboy };
  TimerController timer{ &gameboy, &bus };

  SUBCASE("Initial State") {
    // Check initial DIV register value (0xFF04)
    CHECK_EQ(bus.read(0xFF04), 0x00);

    // Check initial TIMA register value (0xFF05)
    CHECK_EQ(bus.read(0xFF05), 0x00);

    // Check initial TMA register value (0xFF06)
    CHECK_EQ(bus.read(0xFF06), 0x00);

    // Check initial TAC register value (0xFF07)
    CHECK_EQ(bus.read(0xFF07), 0x00);
  }
}

TEST_CASE("TimerController Clock Operations") {
  Gameboy gameboy{ std::vector<word>(0x8000, 0) };
  AddressBus bus{ &gameboy };
  TimerController timer{ &gameboy, &bus };

  SUBCASE("DIV Register Increment") {
    // I don't think Doctest guarantees that tests happen in order. So, I must read initial value first
    word initialDiv = bus.read(0xFF04);

    // DIV increments every 64 cycles
    for (int i = 0; i < 64; i++) {
      timer.machineClock();
    }

    CHECK_EQ(bus.read(0xFF04), initialDiv + 1);
  }

  SUBCASE("TIMA Register with Different Rates") {
    word initialTima;
    // Test TIMA increment with different clock rates
    // TAC register bits 0-1 control the rate:
    // 00: 4096 Hz (256 cycles)
    // 01: 262144 Hz (4 cycles)
    // 10: 65536 Hz (16 cycles)
    // 11: 16384 Hz (64 cycles)

    // Enable timer and set rate to 4 cycles
    bus.write(0xFF07, 0b101);  // Enable timer (bit 2) and set rate to 01
    initialTima = bus.read(0xFF05);
    // Run for 4 cycles
    for (int i = 0; i < 4; i++) {
      timer.machineClock();
    }
    CHECK_EQ(bus.read(0xFF05), initialTima + 1);

    // Enable timer and set rate to 16 cycles
    bus.write(0xFF07, 0b110);  // Enable timer (bit 2) and set rate to 10
    initialTima = bus.read(0xFF05);
    // Run for 16 cycles
    for (int i = 0; i < 16; i++) {
      timer.machineClock();
    }
    CHECK_EQ(bus.read(0xFF05), initialTima + 1);

    // Enable timer and set rate to 64 cycles
    bus.write(0xFF07, 0b111);  // Enable timer (bit 2) and set rate to 11
    initialTima = bus.read(0xFF05);
    // Run for 64 cycles
    for (int i = 0; i < 64; i++) {
      timer.machineClock();
    }
    CHECK_EQ(bus.read(0xFF05), initialTima + 1);

    // Enable timer and set rate to 256 cycles
    bus.write(0xFF07, 0b100);  // Enable timer (bit 2) and set rate to 00
    initialTima = bus.read(0xFF05);
    // Run for 256 cycles
    for (int i = 0; i < 256; i++) {
      timer.machineClock();
    }
    CHECK_EQ(bus.read(0xFF05), initialTima + 1);
  }

  SUBCASE("TIMA Overflow") {
    // Set TIMA to its maximum value
    bus.write(0xFF05, 0xFF);
    // Set TMA (modulo) to a test value
    bus.write(0xFF06, 0x42);
    // Enable timer with fastest rate
    bus.write(0xFF07, 0b101);

    // Run enough cycles to cause overflow
    for (int i = 0; i < 4; i++) {
      timer.machineClock();
    }

    // TIMA should now be equal to TMA after overflow
    CHECK_EQ(bus.read(0xFF05), 0x42);
  }

  SUBCASE("Timer Enable/Disable") {
    // Set initial TIMA value
    bus.write(0xFF05, 0x42);

    // Timer disabled (bit 2 of TAC is 0)
    bus.write(0xFF07, 0b001);  // Rate 01 but timer disabled

    // Run enough cycles that would normally cause increment
    for (int i = 0; i < 512; i++) {
      timer.machineClock();
    }

    // TIMA should not change when timer is disabled
    CHECK_EQ(bus.read(0xFF05), 0x42);
  }
}

TEST_CASE("TimerController Edge Cases") {
  Gameboy gameboy{ std::vector<word>(0x8000, 0) };
  AddressBus bus{ &gameboy };
  TimerController timer{ &gameboy, &bus };

  SUBCASE("DIV Reset") {
    // Run some cycles to increment DIV
    for (int i = 0; i < 128; i++) {
      timer.machineClock();
    }

    // Writing any value to DIV resets it to 0
    bus.write(0xFF04, 0x42);
    CHECK_EQ(bus.read(0xFF04), 0x00);
  }
}