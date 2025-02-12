#include "gb-component.hpp"

namespace gb {

GBComponent::GBComponent(Gameboy* gameboy, AddressBus* bus)
  : bus{ bus }
  , gameboy{ gameboy } {}

}