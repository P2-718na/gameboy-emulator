#ifndef GB_COMPONENT_H
#define GB_COMPONENT_H

namespace gb {

class AddressBus;
class Gameboy;

class GBComponent {

 protected:
  // Fixme these raw pointers should go
  AddressBus* bus;
  Gameboy* gameboy;

  GBComponent(Gameboy* gameboy, AddressBus* bus);
  virtual ~GBComponent() = default;

  virtual void machineClock() = 0;
};

}

#endif  // GB_COMPONENT_H
