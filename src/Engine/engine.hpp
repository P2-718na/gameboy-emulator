#ifndef GAMEBOY_ENGINE_HPP
#define GAMEBOY_ENGINE_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "config.hpp"
#include "gameboy.hpp"

#include <chrono>
#include <functional>

namespace gb {

// TODO
class Engine {
  static constexpr int displayInterval_{016742}; // us
  static constexpr int machineClockInterval_{950}; // ns

  // SFML-related members
  sf::RenderWindow window_;
  Gameboy& gameboy_;
  sf::Texture texture_;
  sf::Sprite sprite_;

  std::chrono::time_point<std::chrono::system_clock> startTime_;

  // Handle all sfml events.
  void handleEvent_(const sf::Event& event);

  static void clockMachine(Engine* ptr);
  void updateTexture();
  void drawScreen();

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  Engine(Gameboy& gameboy);
  //////////////////////////////////////////////////////////////////////////////

  // Print a message to the console.
  // todo mark this function as debug so that it does not get compiled in release build
  static void debug(const std::string& message) noexcept;

  // Todo make with std::function
  void start();
  static void setInterval(
    std::function<void(Engine*)> func, Engine* ptr, unsigned int microseconds);

  // Todo understand thread stuff
};

}  // namespace gb

#endif  // define GAMEBOY_ENGINE_HPP
