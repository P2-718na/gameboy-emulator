#include "frontend.hpp"
#include "doctest.h"

using namespace gb;

TEST_CASE("Frontend ROM Loading") {
  SUBCASE("Invalid ROM Path") {
    CHECK_THROWS(Frontend("nonexistent.gb"));
    CHECK_THROWS(Frontend::getROM("nonexistent.gb"));
  }
}