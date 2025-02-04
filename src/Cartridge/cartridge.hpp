#ifndef CARTRIDGE_H
#define CARTRIDGE_H

namespace gb {

typedef enum {
  ROMONLY,
  MBC1,
  MBC2,
  MBC3,
  MBC5,
  Rumble,
  HUC1
} CartridgeType;

class Cartridge {

  CartridgeType type_{ROMONLY};

  public:
   // todo add proper constructor for different cart types
   Cartridge();

   CartridgeType type() const;
};

}

#endif //CARTRIDGE_H
