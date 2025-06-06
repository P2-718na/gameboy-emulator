#include "gameboy.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "address-bus.hpp"
#include "cartridge.hpp"
#include "cartridge-types.hpp"

namespace gb {

/**
 * Requests an interrupt to the CPU by enabling its corresponding flag to true.
 * @param interrupt ID (Interrupt flag bit) of the interrupt to request.
 */
void Gameboy::requestInterrupt(INTERRUPT_ID interrupt) {
  cpu.IF(interrupt, true);
}

// Constructor /////////////////////////////////////////////////////////////////
/**
 * Instantiate the emulator with given ROM inserted. Emulation will start at
 * $0000 and go through the boot rom; then, it will load the game
 * (exactly as the original hardware did).
 * @param rom Binary ROM of a gameboy game. Has to be one of MBC0, MBC1 or MBC3
 * types. It has to be a valid gameboy ROM (only some basic checks are
 * performed).
 * @throws ROM binary is either invalid data or of a unsupported MBC type.
 */
Gameboy::Gameboy(const Binary& rom) {
  const auto mbc = Cartridge::getMBC(rom);

  switch (mbc) {
    case Cartridge::MBC0:
      cart = std::make_unique<MBC0>(rom);
      break;

    case Cartridge::MBC1:
    case Cartridge::MBC1_RAM:
    case Cartridge::MBC1_RAM_BATTERY:
      cart = std::make_unique<MBC1>(rom);
      break;

    case Cartridge::MBC3:
    case Cartridge::MBC3_RAM_BATTERY:
      cart = std::make_unique<MBC3>(rom);
      break;

    // These are unsupported for now
    // (Even if a timer implementation exists, it is unclear how
    //  some writes should be handled by the cartridge).
    case Cartridge::MBC3_TIMER_BATTERY:
    case Cartridge::MBC3_TIMER_RAM_BATTERY:
    default:
      throw std::runtime_error("Unsupported or invalid MBC type. "
        "Check that the ROM you are using is valid and supported.");
  }

  // This does not transfer ownership of cartridge to AddressBus.
  // Cartridge is owned by Gameboy.
  // I do not use a shared ptr here as when Gameboy dies then AddressBus dies as well.
  bus.loadCart(cart.get());
  isCartridgeBatteryBacked = cart->getHeader().isBatteryBacked;
}

// Methods /////////////////////////////////////////////////////////////////////
/*
 * Step the whole system one machine clock. This function has to be called with
 * a frequency of exactly 1048576Hz if one wants to emulate the system at the
 * correct speed. The frequency can be changed to alter emulation speed.
 */
void Gameboy::machineClock() {
  // TODO one nice feature one could add is to run each component in its
  //  separate thread, in order to speed up emulation. There should be
  //  no problems/race conditions as real hardware worked just like that.
  tcu.machineClock();
  cpu.machineClock();
  ppu.machineClock();
}

/**
 * Skips execution to the end of boot ROM and disables it. Tries to
 * initialize all registers and address bus addresses to their correct values.
 * The documentation states that these values should not be relied upon by ROMs
 * and that they may not be universal across devices.
 */
void Gameboy::skipBoot() {
  // Disable BOOT ROM
  bus.write(BOOT_ROM_LOCK, 0b1);
  assert(!bus.isBootRomEnabled());

  // Set registers in RAM. This is intended mostly for
  // testing purposes. Audio registers are NOT set properly here
  // as emulator still has no audio.
  // TODO set audio registers.
  bus.write(REG_JOIP, 0xCF, AddressBus::GB);
  bus.write(REG_SB,   0x00, AddressBus::GB);
  bus.write(REG_SC,   0x7E, AddressBus::GB);
  bus.write(REG_DIV,  0x18, AddressBus::GB);
  bus.write(REG_TIMA, 0x00, AddressBus::GB);
  bus.write(REG_TMA,  0x00, AddressBus::GB);
  bus.write(REG_TAC,  0xF8, AddressBus::GB);
  bus.write(REG_IF,   0xE1, AddressBus::GB);
  bus.write(REG_LCDC, 0x91, AddressBus::GB);
  bus.write(REG_SCY,  0x00, AddressBus::GB);
  bus.write(REG_SCX,  0x00, AddressBus::GB);
  bus.write(REG_DMA,  0xFF, AddressBus::GB);
  bus.write(REG_BGP,  0xFC, AddressBus::GB);
  bus.write(REG_WY,   0x00, AddressBus::GB);
  bus.write(REG_WX,   0x00, AddressBus::GB);
  bus.write(REG_IE,   0x00, AddressBus::GB);

  // Set CPU Registers to their state after boot rom.
  cpu.reset();
}

/**
 * Loads save binary into battery-backed RAM.
 * This funciton should only be called if the cartridge effectively supports
 * saving (check shouldSave()).
 * @param ram Binary data containing battery-backed external RAM.
 * @throws RAM data is of a different size than the cartridge expects.
 */
void Gameboy::loadSave(const Binary& ram) {
  if (!isCartridgeBatteryBacked) {
    throw std::runtime_error("Attempting to load a save game for a cartridge that is not battery-backed!");
  }
  cart->loadBatteryBackedRAM(ram);
}

/**
 * Set which buttons are pressed. This should be called whenever an input
 * changes (rising/falling edge of a button press). If called with no changes,
 * it does nothing.
 * @param value Bitset containing the current status of the controls.
 * Bits 7 to 0 encode, in order:
 * START, SELECT, B, A, down, up, left, right
 * A value of 0 means that a button is pressed.
 */
void Gameboy::setJoypad(word value) {
  // From the docs:
  // Here value contains wether or not the inputs are pressed.
  // This should be called only on a rising/falling edge of one interrupt,
  // otherwise the call will be ignored and the interrupt not fired.
  // A bit of 1 means that the button is NOT pressed. value must contain on the
  // lower nibble the values for the D-PAD and in the higher nibble the values
  // for the buttons.

  // Ignore call if it doesn't change any inputs.
  if (joypadStatus == value) {
    // If execution reaches here, it could be classified as a mistake in the implementation of
    // the frontend, but I feel like it can also just be an implementation choice
    // that makes things easier. So, no failed asserts here.
    return;
  }

  // Essentially joypadStatus is used only to check if the input state has
  // changed.
  joypadStatus = value;
  bus.write(REG_JOIP, value, AddressBus::GB);
  requestInterrupt(INTERRUPT_JOYPAD);
}

/**
 * Check if the display is enabled or disabled.
 * @return Display status (true = 0n).
 */
bool Gameboy::isScreenOn() const {
  return ppu.LCDC(PPU::LCD_DISPLAY_ENABLE);
}

/**
 * Print the screen buffer using ASCII characters. This is intended to be used
 * for debugging purposes.
 */
void Gameboy::printScreenBuffer() const {
  static constexpr std::array<char, 4> ASCIIColors{'.', 'o', '#', '@'};

  for (int y = 0; y != PPU::HEIGHT; ++y) {
    for (int x = 0; x != PPU::WIDTH; ++x) {
      const auto pixel =
        ASCIIColors[screenBuffer[x + y * PPU::WIDTH].to_ulong()];
      std::cout << pixel;
    }

    std::cout << std::endl;
  }
}

/**
 * Print serial buffer to console. This is intended to be used for
 * debugging purposes, as serial communications is not
 * properly implemented yet.
 */
void Gameboy::printSerialBuffer() {
  if (serialBuffer.empty()) {
    return;
  }

  // The code is used to color the terminal output. It should be supported
  // by all major terminals.
  // We also pass std::flush to make sure the serial data gets printed.
  std::cout << "\092[0m" << serialBuffer << std::flush;
  serialBuffer.clear();
}

/**
 * Get the data present in the battery-backed portion of the RAM.
 * This funciton should only be called if the cartridge effectively supports
 * saving (check shouldSave()).
 * @return battery-backed cartridge RAM (as an array of word(s)).
 */
const Binary& Gameboy::getSave() {
  assert(isCartridgeBatteryBacked);
  return cart->getRam();
}

/**
 * Check wether the cartridge supports save games or not.
 * @return should you care about save games?
 */
bool Gameboy::shouldSave() const {
  return isCartridgeBatteryBacked;
}

} // namespace gb