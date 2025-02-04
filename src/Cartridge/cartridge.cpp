#include "cartridge.hpp"

#include <cassert>

namespace gb {

Cartridge::Cartridge() = default; // Todo delete default constructor and make sure this loads a rom

CartridgeType Cartridge::type() const {
  return type_;
}

}