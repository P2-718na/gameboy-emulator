#include "gb-component.hpp"

namespace gb {

// fixme some stuff about virtual destructor idk
GBComponent::GBComponent(Gameboy* gameboy, AddressBus* bus)
  : bus{ bus }
  , gameboy{ gameboy } {}

}