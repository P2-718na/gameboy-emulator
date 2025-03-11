#ifndef GAMEBOY_FRONTEND_HPP
#define GAMEBOY_FRONTEND_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <chrono>

#include "gameboy.hpp"
#include "ppu.hpp"
#include "types.hpp"

namespace gb {

class Frontend {
  // To preserve vertical sync, we update once every full PPU cycle.
  static constexpr int displayInterval{17556}; // Machine clocks for a full draw
  static constexpr int nanosecondsPerClock{static_cast<int>(1./1048576. * 1E9)};
  static constexpr std::chrono::nanoseconds machineClockInterval{nanosecondsPerClock}; // Clock runs at 1048576 MHz

  // Game Boy screen dimensions (160x144)
  static constexpr int width{PPU::WIDTH};
  static constexpr int height{PPU::HEIGHT};

  // Parameters for color display.
  static constexpr int colorChannels{4};
  static constexpr int maxColorDepth{3};
  static constexpr int shadeWidth{50};

  // Emulator library. Gets initialized in constructor.
  Gameboy gameboy;
  std::string savePath{};

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

  bool capSpeed{true};

  // Handle all SFML window events.
  void handleEvent(const sf::Event& event);

  // Update texture, sprite and screen each frame.
  void updateTexture();
  void drawScreen();

  void loadSave();
  void mainLoop();

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  explicit Frontend(const std::string& romPath);
  //////////////////////////////////////////////////////////////////////////////

  // Start emulation loop
  void start();

  // Load ROM data from file.
  static Binary getROM(const std::string& romPath);

  void saveGame();
};

}  // namespace gb

#endif  // define GAMEBOY_FRONTEND_HPP
