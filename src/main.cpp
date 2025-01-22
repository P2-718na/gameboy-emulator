#include <iostream>
#include <string>
#include <lyra/lyra.hpp>

#include "engine.hpp"
#include "gameboy.hpp"

int main(int argc, char* argv[]) {
  //bool showHelp{false};
  //
  ///* clang-format off */
  //auto cli = lyra::help(showHelp)
  //  | lyra::opt(entitiesPath, "entities file")
  //    ["-e"]["--entities"]
  //    ("Entities file (generated using background image).")
  //    .required();
  ///* clang-format on */
  //
  //auto result = cli.parse({ argc, argv });
  //
  //// CLion doesn't recognize that lyra parser can set showHelp to true.
  //// Ignore "condition is always false" warning.
  //if (showHelp || argc == 1) {
  //  std::cout << cli;
  //  exit(EXIT_SUCCESS);
  //}
  //
  //if (!result) {
  //  std::cerr << "Error in arguments: " << result.errorMessage() << std::endl;
  //  exit(EXIT_FAILURE);
  //}

  try {
    gb::Gameboy gameboy;

    gb::Engine engine(gameboy);
    gb::Engine::debug("Starting emulator...");

    gameboy.turnOn();

  } catch (std::runtime_error& err) {
    std::cerr << "An error occurred while initializing the emulator:"
              << std::endl << err.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    std::cerr << "Something went horribly wrong! Terminating..." << std::endl;
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
