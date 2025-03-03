#ifndef GAMEBOY_FRONTEND_HPP
#define GAMEBOY_FRONTEND_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <chrono>

#include "gameboy.hpp"
#include "types.hpp"


namespace gb {

class Frontend {
  static constexpr int displayInterval_{16742}; // us
  static constexpr int machineClockInterval_{1}; // ns, 950

  // SFML-related members
  sf::RenderWindow window_;
  Gameboy gameboy_;
  sf::Texture texture_;
  sf::Sprite sprite_;

  std::chrono::time_point<std::chrono::system_clock> startTime_;

  // Handle all sfml events.
  void handleEvent_(const sf::Event& event);
  static const Binary loadRom(const std::string& romPath);
  void updateTexture();
  void drawScreen();

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  explicit Frontend(const std::string& romPath);
  //////////////////////////////////////////////////////////////////////////////

  void start();
};

}  // namespace gb

#endif  // define GAMEBOY_FRONTEND_HPP
