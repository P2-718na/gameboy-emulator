#include "frontend.hpp"
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace gb {

// Constructor /////////////////////////////////////////////////////////////////
Frontend::Frontend(
  Gameboy& gameboy)
  : gameboy_(gameboy) {
  texture_.create(160, 144);
  sprite_.setTexture(texture_);
}

// Methods /////////////////////////////////////////////////////////////////////
void Frontend::debug(const std::string& message) noexcept {
  std::cout << message << std::endl;
}

void Frontend::handleEvent_(const sf::Event& event) {
  if (event.type == sf::Event::Closed) {
    // Close window. This will end the loop and close simulation.
    window_.close();
    //exit(0);
  }

  // Handle key presses
  if (event.type == sf::Event::KeyReleased || event.type == sf::Event::KeyPressed) {
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

    gameboy_.setJoypad(joypad.to_ulong());
  }
}

void Frontend::clockMachine(Frontend* ptr) {
  ptr->gameboy_.machineClock();
}

void Frontend::updateTexture() {
  const int width = 160;
  const int height = 144;

  const auto buffer = gameboy_.screenBuffer;

  // todo this is from official docs, maybe fix
  auto* pixels = new sf::Uint8[width * height * 4];
  for (int i = 0; i != (width * height); ++i) {
    const int pos = i * 4;
    pixels[pos+0] = (3-buffer[i].to_ulong()) * 50;
    pixels[pos+1] = (3-buffer[i].to_ulong()) * 50;
    pixels[pos+2] = (3-buffer[i].to_ulong()) * 50;
    pixels[pos+3] = 255;
  }
  texture_.update(pixels);
  //todo yeah this sucks
  delete[] pixels;
}

void Frontend::drawScreen() {
  window_.clear();

  updateTexture();
  //gameboy_.ppu_.printBuffer();

  // set the shape color to green
  window_.draw(sprite_);

  // Display window.
  window_.display();

}

void Frontend::setInterval(std::function<void(Frontend*)> func, Frontend* ptr, unsigned int nanoseconds) {
  std::thread([func, ptr, nanoseconds]() -> void {
   auto lastClockTime = std::chrono::high_resolution_clock::now();
   while (true) {
     while (std::chrono::high_resolution_clock::now() < lastClockTime + std::chrono::nanoseconds(nanoseconds));
     func(ptr);
     lastClockTime = std::chrono::high_resolution_clock::now();
     //std::this_thread::sleep_until() bugged asf
   }
 }).detach();
}


void Frontend::start() {
  const sf::VideoMode videoMode{ 160,
                                 144 };
  window_.create(videoMode, "GameBoy");

  auto lastDrawTime = std::chrono::high_resolution_clock::now();
  setInterval(clockMachine, this, machineClockInterval_);

  sf::Event event;
  while (window_.isOpen()) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    if (currentTime - lastDrawTime > std::chrono::microseconds(16000)) {
      while (window_.pollEvent(event)) {
        handleEvent_(event);
      }

      drawScreen();
      gameboy_.printSerialBuffer();
      lastDrawTime = currentTime;
    }
  }
}


}  // namespace pandemic
