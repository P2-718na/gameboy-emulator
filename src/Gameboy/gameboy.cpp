#include "gameboy.hpp"
#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include "address-bus.hpp"
#include "cpu.hpp"
#include "cartridge.hpp"
#include "cartridge-types.hpp"

namespace gb {

void Gameboy::requestInterrupt(FlagInterrupt interrupt) {
  cpu.IF(interrupt, true);
}

// Constructors /////////////////////
Gameboy::Gameboy(const std::vector<word>& rom) {
  const auto MBCType = Cartridge::getMBC(rom);

  // Todo this code should be refactored.
  // Cartridge should just be a normal class,
  // and I should add the abstract class "Controlled", with all its derivatees MBC0, MBC1...
  // Since I am supporting only a few cartridges, this is fine for now.
  switch (MBCType) {
    case Cartridge::MBC0:
      // Todo smart pointers
      cart = new MBC0{rom};
      break;

    case Cartridge::MBC1:
    case Cartridge::MBC1_RAM:
    case Cartridge::MBC1_RAM_BATTERY:
      cart = new MBC1{rom};
      break;

    case Cartridge::MBC3:
    case Cartridge::MBC3_RAM_BATTERY:
    case Cartridge::MBC3_TIMER_BATTERY:
    case Cartridge::MBC3_TIMER_RAM_BATTERY:
      cart = new MBC3{rom};
      break;

    default:
      throw std::runtime_error("Unsupported or invalid MBC type. Check that the ROM you are using is valid and supported.");
  }

  bus.loadCart(cart);
}

// Public /////////////////////////////////////////////////////////
void Gameboy::machineClock() {
  timers.machineClock();
  cpu.machineClock();
  ppu.machineClock();
}

// getters
const Gameboy::ScreenBuffer& Gameboy::getScreenBuffer() const {
  return screenBuffer;
}


bool Gameboy::isScreenOn() const {
  return ppu.LCDC(PPU::LCD_Display_Enable);
}

// Debug
void Gameboy::printScreenBuffer() const {
  static constexpr std::array<char, 4> grayscale{'.', 'o', '#', '@'};

  for (int y = 0; y != 144; ++y) {
    for (int x = 0; x != 160; ++x) {
      const auto pixel = grayscale[screenBuffer[x + y * PPU::width_].to_ulong()];
      std::cout << pixel;
    }

    std::cout << std::endl;
  }
}

void Gameboy::printSerialBuffer() const {
  std::cout << "\092[0m" << serialBuffer;
}


} // namespace gb