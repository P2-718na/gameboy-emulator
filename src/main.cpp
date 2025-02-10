#include <iostream>
#include <string>
#include <lyra/lyra.hpp>

#include "engine.hpp"
#include "gameboy.hpp"

int main(int argc, char* argv[]) {

  // Todo handle errors
  gb::Gameboy gameboy{"test.gb"};
  gb::Engine engine{gameboy};


  engine.start();

  return EXIT_SUCCESS;
}
