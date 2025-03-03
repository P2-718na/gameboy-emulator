#include <iostream>
#include <string>
#include <lyra/lyra.hpp>

#include "frontend.hpp"
#include "gameboy.hpp"

int main(int argc, char* argv[]) {

  // Parse command line arguments
  bool showHelp{false};
  std::string romPath{};

  const auto cli = lyra::help(showHelp)
                 | lyra::arg(romPath, "path")
                   ("Path to Game Boy rom.");

  const auto result = cli.parse({ argc, argv });

  if (!result)
  {
    std::cerr << result.errorMessage() << std::endl;
    std::cerr << cli;
    exit(EXIT_FAILURE);
  }

  if(showHelp)
  {
    std::cout << cli << '\n';
    exit(EXIT_SUCCESS);
  }

  // Load and run engine
  try {
    // try/catch used to have an impact on performance, but now most compiler
    // handle non-exceptional path without overhead.
    // see https://stackoverflow.com/questions/16784601/does-try-catch-block-decrease-performance
    gb::Frontend frontend{ romPath };
    frontend.start();
  } catch (const std::runtime_error& err) {
    std::cerr << "An unexpected error occurred while setting up the emulator:" << std::endl;
    std::cerr << err.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
