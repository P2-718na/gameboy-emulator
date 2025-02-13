// Doctest requires this define to be specified only once.
// Probably this is the best place
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"

TEST_CASE("Check that doctest is working") {
  REQUIRE(true);
}
