#ifndef GAMEBOY_CONFIG_HPP
#define GAMEBOY_CONFIG_HPP

namespace gb {

// This class holds all the configurable variables for the emulaotr.
// For now, it is mostly a placeholder, but it can be expanded in many
// ways.
// E.g. load and write configuration from file, edit emulation parameters
// while the emulator is running, etc...
class Config {

public:
  // Constructor ///////////////////////////////////////////////////////////////
  // Initialize generator to catch errors.
  Config();
};

}  // namespace gb

#endif  // define GAMEBOY_CONFIG_HPP
