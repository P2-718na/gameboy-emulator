#ifndef GAMEBOY_ENGINE_HPP
#define GAMEBOY_ENGINE_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "gameboy.hpp"

#include <chrono>
#include <functional>

namespace gb {

class Frontend {
  static constexpr int displayInterval_{016742}; // us
  static constexpr int machineClockInterval_{1}; // ns, 950

  // SFML-related members
  sf::RenderWindow window_;
  Gameboy& gameboy_;
  sf::Texture texture_;
  sf::Sprite sprite_;

  std::chrono::time_point<std::chrono::system_clock> startTime_;

  // Handle all sfml events.
  void handleEvent_(const sf::Event& event);

  void updateTexture();
  void drawScreen();

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  explicit Frontend(Gameboy& gameboy);
  //////////////////////////////////////////////////////////////////////////////

  // Print a message to the console.
  // todo mark this function as debug so that it does not get compiled in release build
  static void debug(const std::string& message) noexcept;

  // Todo make with std::function
  void start();

  // Todo understand thread stuff
};

}  // namespace gb

#endif  // define GAMEBOY_ENGINE_HPP
