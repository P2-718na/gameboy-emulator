#include <cassert>
#include <chrono>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "frontend.hpp"

namespace gb {

constexpr int Frontend::displayInterval;
constexpr int Frontend::nanosecondsPerClock;
constexpr std::chrono::nanoseconds Frontend::machineClockInterval;
constexpr int Frontend::width;
constexpr int Frontend::height;
constexpr int Frontend::colorChannels;
constexpr int Frontend::maxColorDepth;

// Constructor /////////////////////////////////////////////////////////////////
Frontend::Frontend(const std::string& romPath)
  : gameboy{ getROM(romPath)}
{
  texture.create(160, 144);
  sprite.setTexture(texture);

  savePath = romPath + ".sav";
  loadSave();
}

// Methods /////////////////////////////////////////////////////////////////////
void Frontend::handleEvent(const sf::Event& event) {
  if (event.type == sf::Event::Closed) {
    // Close window. This will end the loop and close simulation.
    window.close();
    saveGame();
    exit(EXIT_SUCCESS);
  }

  // Handle key presses
  const bool keyChanged = event.type == sf::Event::KeyReleased
                       || event.type == sf::Event::KeyPressed;
  if (!keyChanged) {
    return;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
    saveGame();
    return;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
    capSpeed = !capSpeed;
    return;
  }

  // Update joystick bitset
  const bool startUp  = !sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);
  const bool selectUp = !sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
  const bool BUp      = !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
  const bool AUp      = !sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
  const bool dArrowUp = !sf::Keyboard::isKeyPressed(sf::Keyboard::S);
  const bool uArrowUp = !sf::Keyboard::isKeyPressed(sf::Keyboard::W);
  const bool lArrowUp = !sf::Keyboard::isKeyPressed(sf::Keyboard::A);
  const bool rArrowUp = !sf::Keyboard::isKeyPressed(sf::Keyboard::D);

  std::bitset<8> joypad;
  joypad[7] = startUp;
  joypad[6] = selectUp;
  joypad[5] = BUp;
  joypad[4] = AUp;
  joypad[3] = dArrowUp;
  joypad[2] = uArrowUp;
  joypad[1] = lArrowUp;
  joypad[0] = rArrowUp;

  gameboy.setJoypad(joypad.to_ulong());
}

Binary Frontend::getROM(const std::string& romPath) {
  std::ifstream input(romPath, std::ios_base::binary);

  if (input.fail()) {
    throw std::runtime_error("Error reading ROM file!");
  }

  // Copy all data to ROM, it's faster than reading from file each time.
  const auto rom = Binary{std::istreambuf_iterator<char>(input), {}};

  return rom;
}

void Frontend::loadSave() {
  if (!gameboy.shouldSave()) {
    return;
  }

  std::ifstream input(savePath, std::ios_base::binary);

  if (input.fail()) {
    std::cout << "No save file to load." << std::endl;
    return;
  }

  std::cout << "Save file loaded!" << std::endl;
  const auto ram = Binary{std::istreambuf_iterator<char>(input), {}};

  gameboy.loadSave(ram);
  gameboy.skipBoot();
}

void Frontend::updateTexture() {
  // We need to copy this data to RGB format.
  const auto buffer = gameboy.screenBuffer;

  // This is an arbitrary conversion. It looks nice this way.
  for (int bufferPosition = 0; bufferPosition != (width * height); ++bufferPosition) {
    const int pixelPosition = bufferPosition * colorChannels;
    const auto currentColor = buffer[bufferPosition].to_ulong();

    // Each different color is a shade of gray
    pixels[pixelPosition + 0] = (maxColorDepth - currentColor) * shadeWidth;
    pixels[pixelPosition + 1] = (maxColorDepth - currentColor) * shadeWidth;
    pixels[pixelPosition + 2] = (maxColorDepth - currentColor) * shadeWidth;

    // We set opacity to max (NO opacity)
    pixels[pixelPosition + 3] = 255;
  }
  texture.update(pixels);
}

void Frontend::drawScreen() {
  // SFML docs recommend to clear window even if it will be overwritten
  window.clear();
  updateTexture();
  // sprite does not need to be updated as it hold a reference to texture.
  // Also, sprite gets drawn by default at 0, 0 which is perfect.
  window.draw(sprite);

  // Push changes to screen
  window.display();
}

void Frontend::saveGame() {
  if (!gameboy.shouldSave()) {
    return;
  }

  const auto ram = gameboy.getSave();
  std::ofstream output(savePath, std::ios_base::binary);
  if (output.fail()) {
    std::cerr << "Error writing save file!" << std::endl;
    return;
  }

  // Copy all data to ROM, it's faster than reading from file.
  output.write((char*)&ram[0], static_cast<int>(ram.size()));
}

void Frontend::mainLoop() {
  sf::Event event{};
  while (window.isOpen()) {
    const auto currentTime = std::chrono::steady_clock::now();
    const auto timeDelta = currentTime - lastClockTime;
    // Only clock machine if enough time has passed.
    const bool shouldClockMachine = timeDelta > machineClockInterval || !capSpeed;

    // The display will get drawn only after a clock has occurred.
    if (!shouldClockMachine) {
      continue;
    }

    gameboy.machineClock();
    ++cyclesSinceLastDraw;
    lastClockTime = currentTime;

    const bool shouldDrawDisplay = cyclesSinceLastDraw % displayInterval == 0;
    if (!shouldDrawDisplay) {
      continue;
    }

    // Poll for the next event in queue (window close, button pressed...)
    while (window.pollEvent(event)) {
      handleEvent(event);
    }

    drawScreen();
    gameboy.printSerialBuffer();
    cyclesSinceLastDraw = 0;
  }
}

void Frontend::start() {
  const sf::VideoMode videoMode{ width,
                                 height };
  window.create(videoMode, "GameBoy");

  using namespace std::chrono;

  mainLoop();
}


}  // namespace pandemic
