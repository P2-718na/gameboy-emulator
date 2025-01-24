#include "graphics.hpp"

#include "memory.hpp"

namespace gb {

Graphics::Graphics() = default;


void Graphics::connectMemory(Memory* ram) {
  ram_ = ram;
}

} // namespace gb