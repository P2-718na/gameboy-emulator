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
  // To preserve vertical sync, we update once every full PPU cycle.
  static constexpr int displayInterval{17556}; // Machine clocks for a full draw
  static constexpr int microsecondsPerCLock{static_cast<int>(1./1048576.)};
  static constexpr std::chrono::microseconds machineClockInterval{microsecondsPerCLock}; // Clock runs at 1048576 MHz

  // Gameboy screen dimensions
  static constexpr int width{160};
  static constexpr int height{144};

  // Parameters for color display.
  static constexpr int colorChannels{4};
  static constexpr int maxColorDepth{3};
  static constexpr int shadeWidth{50};

  // Emulator library. Gets initialized in constructor.
  Gameboy gameboy;

  // SFML-related members
  sf::RenderWindow window;
  // Sprite is the emulator LCD. It gets drawn to window and texture holds the
  // data for the screen buffer. (A sprite is needed in order to draw a texture).
  sf::Texture texture;
  sf::Sprite sprite;
  sf::Uint8 pixels[width * height * colorChannels]{};

  // Time in us at which last machine clock was called.
  std::chrono::time_point<std::chrono::steady_clock> lastClockTime{};
  // Number of clocks passed since last screen draw.
  int cyclesSinceLastDraw{0};

  // Handle all SFML window events.
  void handleEvent(const sf::Event& event);

  // Update texture, sprite and screen each frame.
  void updateTexture();
  void drawScreen();

  void mainLoop();

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  explicit Frontend(const std::string& romPath);
  //////////////////////////////////////////////////////////////////////////////

  // Start emulation loop
  void start();

  // Load ROM data from file.
  static Binary loadRom(const std::string& romPath);
};

}  // namespace gb

#endif  // define GAMEBOY_FRONTEND_HPP
