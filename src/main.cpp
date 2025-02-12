#include <iostream>
#include <string>
#include <lyra/lyra.hpp>

#include "frontend.hpp"
#include "gameboy.hpp"

int main(int argc, char* argv[]) {

  // Todo handle errors
  gb::Gameboy gameboy{"test.gb"};
  gb::Frontend engine{gameboy};


  engine.start();

  return EXIT_SUCCESS;
}
