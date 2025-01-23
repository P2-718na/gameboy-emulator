#ifndef GAMEBOY_ENGINE_HPP
#define GAMEBOY_ENGINE_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "config.hpp"
#include "gameboy.hpp"

namespace gb {

// TODO
class Engine {
  // SFML-related members
  sf::RenderWindow window_;

  // Hold simulation world and config.
  Config config_{};

  Gameboy& gameboy_;

  // Handle all sfml events.
  void handleEvent_(const sf::Event& event) noexcept;

  // Display the simulation. Gets called based on refresh rate.
  void graphicsLoop_() noexcept;

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  Engine(Gameboy& gameboy);
  //////////////////////////////////////////////////////////////////////////////

  // Print a message to the console.
  // todo mark this function as debug so that it does not get compiled in release build
  static void debug(const std::string& message) noexcept;
};

}  // namespace gb

#endif  // define GAMEBOY_ENGINE_HPP
