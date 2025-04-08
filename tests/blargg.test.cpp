#include <fstream>
#include <iostream>
#include <string>
#include "cartridge.hpp"
#include "cpu.hpp"
#include "doctest.h"
#include "gameboy.hpp"
#include "types.hpp"

using namespace gb;
using std::string;

// Check if a string ends with a given substring.
bool endsWith(const string& fullString, const string& ending) {
  // Check if the ending string is longer than the full
  // string
  if (ending.size() > fullString.size())
    return false;

  // Compare the ending of the full string with the target
  // ending
  return fullString.compare(fullString.size() - ending.size(), ending.size(), ending) == 0;
}

// Run a single test ROM for a certain number of CPU cycles.
// Returns true if the test passes, false otherwise.
// Checking that the test passes is done through the serial output.
bool runSingleTestForNCycles(string romPath, int cycles) {
  std::ifstream input(romPath, std::ios_base::binary);
  REQUIRE_FALSE(input.fail());

  const auto rom = gb::Binary(std::istreambuf_iterator<char>(input), {});

  gb::Gameboy gameboy{ rom };
  gameboy.skipBoot();

  while (cycles --> 0) {
    gameboy.machineClock();

    if (endsWith(gameboy.serialBuffer, "Passed")) {
      return true;
    }

    if (endsWith(gameboy.serialBuffer, "Failed")) {
      // gameboy.printSerialBuffer();
      return false;
    }
  }

  return false;
}

// Some of Blargg's test roms WILL fail (e.g. sub cycle memory timing).
// I included these in the tests just as placeholders. Here, I check that they fail.
// The important thing for these tests is that they do not cause any memory issue
// and that they don't crash the program.

// Blargg's cpu instruction test ROM executes fine. This is the most important test
// And we want it to succeed.
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

// This fails with code #255.
// It appears that to fix this, I would need to update internal timer
// With sub-machine-clock precision.
// (see https://github.com/feo-boy/feo-boy/blob/master/src/bus/timer.rs#L56-L66)
TEST_CASE("Blargg instr_timing tests") {
  const string basePath = "blargg-test-roms/instr_timing/";

  CHECK_FALSE(runSingleTestForNCycles(basePath + "instr_timing.gb", 1e7));
}

// This fails probably due to the same reason as instr_timing: sub-machine clock precision
// is needed.
TEST_CASE("Blargg interrupt_time tests") {
  const string basePath = "blargg-test-roms/interrupt_time/";

  CHECK_FALSE(runSingleTestForNCycles(basePath + "interrupt_time.gb", 1e8));
}

// This fails because it is still not implemented.
TEST_CASE("Blargg halt_bug tests") {
  const string basePath = "blargg-test-roms/halt_bug/";

  SUBCASE("halt_bug") {
    CHECK_FALSE(runSingleTestForNCycles(basePath + "halt_bug.gb", 1e7));
  }
}