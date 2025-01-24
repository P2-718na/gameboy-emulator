#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "memory.hpp"

namespace gb {

class Graphics {

public:
  //  Todo same as for processor.hpp
  Memory* ram_;

  // Constructor ///////////////////////////////////////////////////////////////
  Graphics();

  // Todo I dont like this. I'd rather prefer that
  //  this class has a reference to gameboy and that it can call ram from that.
  void connectMemory(Memory* ram);
  //////////////////////////////////////////////////////////////////////////////
};

} // namespace gb

#endif //GAMEBOY_H
