#ifndef MEMORY_H
#define MEMORY_H
#include <array>

#include "types.hpp"
#include "bootrom.hpp"

#include <cartridge.hpp>

namespace gb {

/*
 *Interrupt Enable Register
 --------------------------- FFFF
 Internal RAM
 --------------------------- FF80
 Empty but unusable for I/O
 --------------------------- FF4C
 I/O ports
 --------------------------- FF00
 Empty but unusable for I/O
 --------------------------- FEA0
 Sprite Attrib Memory (OAM)
 --------------------------- FE00
 Echo of 8kB Internal RAM
 --------------------------- E000
 8kB Internal RAM
 --------------------------- C000
 8kB switchable RAM bank
 --------------------------- A000
 8kB Video RAM
 --------------------------- 8000 --
 16kB switchable ROM bank |
 --------------------------- 4000 |= 32kB Cartridge
 16kB ROM bank #0 |
 --------------------------- 0000 --
NOTE: b = bit, B = byte
 */


class Memory {

  const addr BOOT_ROM_LOCK = 0xFF50;


  // TODO think if there is a better way to do this
  const std::array<word, 0x100> bootRom_{BOOTROM_CONTENTS};

  std::array<word, 0xffff> memory_{};
  Cartridge cartridge_{};

  bool isBootRomEnabled();

  public:
   Memory();
   Memory(std::array<word, 0xffff>& memory);

   word read(addr address);
   void write(addr address, word value);
};

}

#endif //MEMORY_H
