#include <string>
#include <fstream>
#include <iostream>
#include "cpu.hpp"
#include "cartridge.hpp"
#include "gameboy.hpp"
#include "doctest.h"
#include "types.hpp"

using namespace gb;
using std::string;

TEST_CASE("Two words to dword type conversion") {
  word msb;
  word lsb;
  dword result;

  msb = 0x9F;
  lsb = 0xFF;
  result = CPU::twoWordToDword(msb, lsb);
  CHECK(result == 0x9FFF);


  msb = 0x0a;
  lsb = 0x9c;
  result = CPU::twoWordToDword(msb, lsb);
  CHECK(result == 0x0a9c);


  msb = 0x00;
  lsb = 0x00;
  result = CPU::twoWordToDword(msb, lsb);
  CHECK(result == 0x0000);
}


TEST_CASE("Dword splitting") {
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

bool endsWith(const string& fullString,
              const string& ending)
{
  // Check if the ending string is longer than the full
  // string
  if (ending.size() > fullString.size())
    return false;

  // Compare the ending of the full string with the target
  // ending
  return fullString.compare(
    fullString.size() - ending.size(),
    ending.size(), ending) == 0;
}

bool runSingleTestForNCycles(string romPath, int cycles) {
  std::ifstream input(romPath, std::ios_base::binary);
  REQUIRE_FALSE(input.fail());

  const auto rom = gb::Cartridge::Rom(std::istreambuf_iterator<char>(input), {});

  // TODO I would really love to implement method to skip boot rom animation
  gb::Gameboy gameboy{rom};
  gameboy.skipBoot();

  while (cycles --> 0) {
    gameboy.machineClock();

    if (endsWith(gameboy.serialBuffer, "Passed")) {
      return true;
    }

    if (endsWith(gameboy.serialBuffer, "Failed")) {
      gameboy.printSerialBuffer();
      return false;
    }
  }

  return false;
}

// Not all roms are used for testing. Mostly because some WILL fail
// (e.g. sub cycle memory timing). Other tests are just used as a warning
// as they should not compromise functionality.
 TEST_CASE("Blargg cpu_instrs tests") {
  const string basePath = "blargg-test-roms/cpu_instrs/individual/";

  SUBCASE("01-special") {
    CHECK(runSingleTestForNCycles(basePath + "01-special.gb", 1e7));
  }
  SUBCASE("02-interrupts") {
    CHECK(runSingleTestForNCycles(basePath + "02-interrupts.gb", 1e7));
  }
  SUBCASE("03-op sp,hl") {
    CHECK(runSingleTestForNCycles(basePath + "03-op sp,hl.gb", 1e7));
  }
  SUBCASE("04-op r,imm") {
    CHECK(runSingleTestForNCycles(basePath + "04-op r,imm.gb", 1e7));
  }
  SUBCASE("05-op rp") {
    CHECK(runSingleTestForNCycles(basePath + "05-op rp.gb", 1e7));
  }
  SUBCASE("06-ld r,r") {
    CHECK(runSingleTestForNCycles(basePath + "06-ld r,r.gb", 1e7));
  }
  SUBCASE("07-jr,jp,call,ret,rst") {
    CHECK(runSingleTestForNCycles(basePath + "07-jr,jp,call,ret,rst.gb", 1e7));
  }
  SUBCASE("08-misc instrs") {
    CHECK(runSingleTestForNCycles(basePath + "08-misc instrs.gb", 1e7));
  }
  SUBCASE("09-op r,r") {
    CHECK(runSingleTestForNCycles(basePath + "09-op r,r.gb", 1e8));
  }
  SUBCASE("10-bit ops") {
    CHECK(runSingleTestForNCycles(basePath + "10-bit ops.gb", 1e8));
  }
  SUBCASE("11-op a,(hl)") {
    CHECK(runSingleTestForNCycles(basePath + "11-op a,(hl).gb", 1e8));
  }

}

TEST_CASE("Blargg instr_timing tests") {
  const string basePath = "blargg-test-roms/instr_timing/";

  SUBCASE("instr_timing") {
    CHECK(runSingleTestForNCycles(basePath + "instr_timing.gb", 1e7));
  }
}

TEST_CASE("Blargg interrupt_time tests") {
  const string basePath = "blargg-test-roms/interrupt_time/";

  SUBCASE("interrupt_time") {
    WARN(runSingleTestForNCycles(basePath + "interrupt_time.gb", 1e8));
  }
}

TEST_CASE("Blargg halt_bug tests") {
  const string basePath = "blargg-test-roms/halt_bug/";

  SUBCASE("halt_bug") {
    WARN(runSingleTestForNCycles(basePath + "halt_bug.gb", 1e8));
  }
}