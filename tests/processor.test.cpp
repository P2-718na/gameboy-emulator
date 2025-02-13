// Doctest requires this define to be specified only once.
// Probably this is the best place
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "cpu.hpp"
#include "doctest.h"
#include "types.hpp"

using namespace gb;

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