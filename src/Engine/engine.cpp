#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "engine.hpp"

namespace gb {

// Constructor /////////////////////////////////////////////////////////////////
Engine::Engine(
  const Gameboy& gameboy)
  : gameboy_{ gameboy } {

  // TODO
  // todo do proper stuff for scaling etc
  const sf::VideoMode videoMode{ 160,
                                 144 };
  window_.create(videoMode, "GameBoy");
}

// Methods /////////////////////////////////////////////////////////////////////
void Engine::debug(const std::string& message) noexcept {
  std::cout << message << std::endl;
}

void Engine::handleEvent_(const sf::Event& event) noexcept {
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

void Engine::graphicsLoop_() noexcept {
  // This is not strictly needed, since we will be redrawing the background
  // which takes up the whole screen, but it's recommended by sfml.
  window_.clear();

  // TODO Draw pixels

  // Display window.
  window_.display();
}

}  // namespace pandemic
