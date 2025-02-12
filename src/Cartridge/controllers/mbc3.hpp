#ifndef CARTRIDGE_MBC3
#define CARTRIDGE_MBC3

namespace gb {

class MBC3 : public Cartridge {

  int romBank{};
  int ramBank{};
  bool modeFlag{false};
  bool externalRamEnabled{false};

 public:
  // TODO
  explicit inline MBC3(const Rom& rom) : Cartridge{rom} { assert(false); };

  inline word read(const dword address) override {
    // Todo assert we are reading address in ROM
    return rom[address];
  }

  inline void write(const dword address, const word value) override {
    // TOdo assert we are writing either to a controller or to
    // An address in RAM
    rom[address] = value;
  }
};

}

#endif