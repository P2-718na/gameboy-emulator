#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <thread>
#include "engine.hpp"
#include <chrono>


namespace gb {

// Constructor /////////////////////////////////////////////////////////////////
Engine::Engine(
  Gameboy& gameboy)
  : gameboy_(gameboy) {
  texture_.create(160, 144);
  sprite_.setTexture(texture_);
}

// Methods /////////////////////////////////////////////////////////////////////
void Engine::debug(const std::string& message) noexcept {
  std::cout << message << std::endl;
}

void Engine::handleEvent_(const sf::Event& event) {
  if (event.type == sf::Event::Closed) {
    // Close window. This will end the loop and close simulation.
    window_.close();
    return;
  }

  // Handle key presses
  if (event.type == sf::Event::KeyReleased) {
    switch (event.key.code) {
      case sf::Keyboard::D:
       debug("Key D pressed");
       break;

      default:
        break;
    }
  }
}

void Engine::clockMachine(Engine* ptr) {
  ptr->gameboy_.clock();
}

void Engine::updateTexture() {
  const int width = 160;
  const int height = 144;

  // todo this is from official docs, maybe fix
  auto* pixels = new sf::Uint8[width * height * 4];
  for (int x = 0; x != width; ++x) {
    for (int y = 0; y != height; ++y) {
      int pos = (y * width + x) * 4;
      pixels[pos+0] = (3-gameboy_.ppu_.screenBuffer_[x][y].to_ulong()) * 50;
      pixels[pos+1] = (3-gameboy_.ppu_.screenBuffer_[x][y].to_ulong()) * 50;
      pixels[pos+2] = (3-gameboy_.ppu_.screenBuffer_[x][y].to_ulong()) * 50;
      pixels[pos+3] = 255;
    }
  }
  texture_.update(pixels);
  //todo yeah this sucks
  delete[] pixels;
}

void Engine::drawScreen() {
  window_.clear();

  updateTexture();
  //gameboy_.ppu_.printBuffer();

  // set the shape color to green
  window_.draw(sprite_);

  // Display window.
  window_.display();

}

void Engine::setInterval(std::function<void(Engine*)> func, Engine* ptr, unsigned int microseconds) {
  std::thread([func, ptr, microseconds]() -> void {
   auto lastClockTime = std::chrono::high_resolution_clock::now();
   while (true) {
     while (std::chrono::high_resolution_clock::now()  < lastClockTime + + std::chrono::microseconds(microseconds));
     func(ptr);
     //std::this_thread::sleep_until() bugged asf
   }
 }).detach();
}


void Engine::start() {
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
      lastDrawTime = currentTime;
    }
  }
}


}  // namespace pandemic
