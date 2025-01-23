// Doctest requires this define to be specified only once.
// Probably this is the best place
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "processor.hpp"
#include "types.hpp"

using namespace gb;

TEST_CASE("Two words to dword type conversion") {
  word msb;
  word lsb;
  dword result;

  msb = 0x9F;
  lsb = 0xFF;
  result = Processor::twoWordToDword(msb, lsb);
  CHECK(result == 0x9FFF);


  msb = 0x0a;
  lsb = 0x9c;
  result = Processor::twoWordToDword(msb, lsb);
  CHECK(result == 0x0a9c);


  msb = 0x00;
  lsb = 0x00;
  result = Processor::twoWordToDword(msb, lsb);
  CHECK(result == 0x0000);
}