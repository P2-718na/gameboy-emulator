#include <cartridge.hpp>
#include <fstream>
#include <iostream>
#include <lyra/lyra.hpp>
#include <string>

#include "frontend.hpp"
#include "gameboy.hpp"
#include "types.hpp"

int main(int argc, char* argv[]) {
  const auto romPath = "test.gb";
  std::ifstream input(romPath, std::ios_base::binary);
  if (input.fail()) {
    throw std::runtime_error("Error reading ROM file!");
  }
  // Copy all data to ROM, it's faster than reading from file.
  const auto rom = gb::Cartridge::Rom(std::istreambuf_iterator<char>(input), {});

  // Todo handle errors
  gb::Gameboy gameboy{rom};
  gb::Frontend engine{gameboy};

  engine.start();

  return EXIT_SUCCESS;
}
