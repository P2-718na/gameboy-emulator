#include "cpu.hpp"
#include "address-bus.hpp"
#include "doctest.h"
#include "gameboy.hpp"
#include "types.hpp"

using namespace gb;

TEST_CASE("CPU Static Utility Functions") {
  SUBCASE("Two words to dword conversion") {
    CHECK_EQ(CPU::twoWordToDword(0x12, 0x34), 0x1234);
    CHECK_EQ(CPU::twoWordToDword(0x00, 0x00), 0x0000);
    CHECK_EQ(CPU::twoWordToDword(0xFF, 0xFF), 0xFFFF);
  }

  SUBCASE("Dword to words conversion") {
    CHECK_EQ(CPU::dwordLsb(0x00FF), 0xFF);
    CHECK_EQ(CPU::dwordLsb(0x0000), 0x00);
    CHECK_EQ(CPU::dwordLsb(0xFF00), 0x00);
    CHECK_EQ(CPU::dwordLsb(0xABCD), 0xCD);
    CHECK_EQ(CPU::dwordLsb(0x0FF0), 0xF0);
    CHECK_EQ(CPU::dwordLsb(0x4321), 0x21);
    CHECK_EQ(CPU::dwordMsb(0x00FF), 0x00);
    CHECK_EQ(CPU::dwordMsb(0x0000), 0x00);
    CHECK_EQ(CPU::dwordMsb(0xFF00), 0xFF);
    CHECK_EQ(CPU::dwordMsb(0xABCD), 0xAB);
    CHECK_EQ(CPU::dwordMsb(0x0FF0), 0x0F);
    CHECK_EQ(CPU::dwordMsb(0x4321), 0x43);
  }

  SUBCASE("Nth bit extraction") {
    word test_byte = 0b10101010;
    CHECK(CPU::nthBit(test_byte, 7));  // MSB should be 1
    CHECK_FALSE(CPU::nthBit(test_byte, 6));
    CHECK(CPU::nthBit(test_byte, 5));
    CHECK_FALSE(CPU::nthBit(test_byte, 4));
    CHECK(CPU::nthBit(test_byte, 3));
    CHECK_FALSE(CPU::nthBit(test_byte, 2));
    CHECK(CPU::nthBit(test_byte, 1));
    CHECK_FALSE(CPU::nthBit(test_byte, 0));  // LSB should be 0
  }

  SUBCASE("Carry flag detection") {
    // Test word carry
    CHECK(CPU::getCarryFlag((word)0xFF, (word)0x01));        // 255 + 1 causes carry
    CHECK_FALSE(CPU::getCarryFlag((word)0x0F, (word)0x0F));  // 15 + 15 doesn't cause carry

    // Test dword carry
    CHECK(CPU::getCarryFlag((dword)0xFFFF, (dword)0x0001));        // 65535 + 1 causes carry
    CHECK_FALSE(CPU::getCarryFlag((dword)0x7FFF, (dword)0x7FFF));  // No carry
  }

  SUBCASE("Half carry flag detection") {
    // Test word half carry (carry from bit 3 to bit 4)
    CHECK(CPU::getHalfCarryFlag((word)0x0F, (word)0x01));        // 15 + 1 causes half carry
    CHECK_FALSE(CPU::getHalfCarryFlag((word)0x0E, (word)0x01));  // 14 + 1 doesn't cause half carry

    // Test dword half carry
    CHECK(CPU::getHalfCarryFlag((dword)0x0FFF, (dword)0x0001));        // 4095 + 1 causes half carry
    CHECK_FALSE(CPU::getHalfCarryFlag((dword)0x0FFE, (dword)0x0001));  // 4094 + 1 doesn't cause half carry
  }

  SUBCASE("Instruction timing cycles") {
    // Test that timing initialization is done properly, Here we only check some random opcodes...
    CHECK(CPU::getBusyCycles(OPCODE::NOP) > 0);   // NOP should take at least 1 cycle
    CHECK(CPU::getBusyCycles(OPCODE::HALT) > 0);  // HALT should take at least 1 cycle

    // Test CB prefixed instruction timings
    CHECK(CPU::getBusyCyclesCB(CB_OPCODE::RLC_B) > 0);    // RLC B should take some cycles
    CHECK(CPU::getBusyCyclesCB(CB_OPCODE::BIT_7_A) > 0);  // BIT 7,A should take some cycles
  }
}

TEST_CASE("CPU Basic Operations") {
  // Run with empty 32KB ROM.
  Gameboy gameboy{ std::vector<word>(0x8000, 0) };

  // Create an additional instance of AddressBus and CPU
  // (Gameboy does not expose either of them, so we must recreate them outside).
  // If we don't call bus.loadCart(...), all of the addresses should return 0xFF.
  AddressBus bus{ &gameboy };
  CPU cpu{ &gameboy, &bus };

  SUBCASE("CPU Reset") {
    cpu.reset();
    // After reset, CPU should be in a known state
    // Right now, checking program counter should be fine.
    // 0x100 is the position at the end of boot ROM (game roms are intended to start here).
    CHECK_EQ(cpu.getPC(), 0x100);
  }

  SUBCASE("Machine Clock") {
    // Test that machine clock advances CPU state
    cpu.reset();
    // reset should set busyCycles to 0.
    CHECK_FALSE(cpu.isBusy());

    cpu.machineClock();

    // Now, CPU is doing something => we expect busyCycles to be > 0.
    CHECK(cpu.isBusy());

    // Wait until instruction has been executed.
    // The CPU was initialized with a null ROM, so it should execute a NOP instruction.
    // (note: BOOT ROM ends just before 0x100).
    while (cpu.isBusy()) {
      cpu.machineClock();
    }

    // 0xFF is the instruction for RST_0x38. So, we check that the program counter is at the right place.
    REQUIRE_EQ(cpu.getPC(), 0x38);
  }
}